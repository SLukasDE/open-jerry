/*
 * This file is part of Jerry application server.
 * Copyright (C) 2020-2021 Sven Lukas
 *
 * Jerry is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Affero General Public License as
 * published by the Free Software Foundation, either version 3 of
 * the License, or (at your option) any later version.
 *
 * Jerry is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Affero General Public License for more details.
 *
 * You should have received a copy of the GNU Affero General Public
 * License along with Jerry.  If not, see <https://www.gnu.org/licenses/>.
 */

#include <jerry/engine/Engine.h>
#include <jerry/engine/ExceptionHandler.h>
#include <jerry/engine/http/server/ExceptionHandler.h>
#include <jerry/Logger.h>

#include <esl/Module.h>
#include <esl/object/Interface.h>
#include <esl/utility/String.h>
#include <esl/system/SignalHandler.h>
#include <esl/Stacktrace.h>

#include <set>
#include <fstream>
#include <vector>
#include <algorithm>
#include <stdexcept>

namespace jerry {
namespace engine {

namespace {
Logger logger("jerry::engine::Engine");
} /* anonymous namespace */

Engine::Engine()
: messageTimer([this](const Message& message) { message.f(*this); })
{ }

bool Engine::run(bool isDaemon) {
	runThreadId = std::this_thread::get_id();

	/* *********************************************************** *
	 * initialize ExceptionHandler:                                *
	 * Load all implementations to convert 'const std::exception&' *
	 * to esl::http::server::exception::Interface::Message         *
	 * *********************************************************** */
	//http::server::ExceptionHandler::initialize();

	/* ************************* *
	 * initialize global objects *
	 * ************************* */
	logger.info << "Initialize context of global objects...\n";
	initializeContext();
	logger.info << "Initialization of global objects done.\n";

	/* *********************************** *
	 * initialize basic-listener objects *
	 * *********************************** */
	logger.info << "Initialize context of all basic-listeners ...\n";
	for(auto& listener : basicListeners) {
		if(listener) {
			listener->initializeContext();
		}
	}
	logger.info << "Initialization of all basic-listeners done.\n";

	/* ******************************** *
	 * initialize http-listener objects *
	 * ******************************** */
	logger.info << "Initialize context of all http-listeners ...\n";
	for(auto& listener : httpListeners) {
		if(listener) {
			listener->initializeContext();
		}
	}
	logger.info << "Initialization of all http-listeners done.\n";

	/* *********************************************************** *
	 * add certificates to socket if http-server is used for https *
	 * *********************************************************** */
	for(auto& entry : httpServerById) {
		http::server::Socket& httpServer = entry.second.get();

		/* add certificates to http-socket if https is used */
		if(httpServer.isHttps()) {
			/* host names for that we need a certificate */
			std::set<std::string> hostnames = httpServer.getHostnames();

			/* add certificate http-socket for each host name */
			for(const auto& hostname : hostnames) {
				auto certIter = certsByHostname.find(hostname);
				if(certIter == std::end(certsByHostname)) {
					throw std::runtime_error("No certificate available for hostname '" + hostname + "'.");
				}
				httpServer.getSocket().addTLSHost(hostname, certIter->second.first, certIter->second.second);
			}
		}
	}

	auto stopFunction = [this]() { stop(); };
	esl::system::SignalHandler::install(esl::system::SignalHandler::SignalType::interrupt, stopFunction);
	esl::system::SignalHandler::install(esl::system::SignalHandler::SignalType::terminate, stopFunction);
	esl::system::SignalHandler::install(esl::system::SignalHandler::SignalType::pipe, stopFunction);

	logger.debug << "Start basic brokers.\n";
	for(auto& entry : basicBrokerById) {
		basic::server::Socket& socket = entry.second.get().getServer();

		logger.debug << "-> Start basic broker \"" << entry.first << "\" for queues:\n";
		if(logger.debug) {
			std::set<std::string> notifiers = socket.getNotifier();
			for(const auto& notifier : notifiers) {
				logger.debug << "   - \"" << notifier << "\"\n";
			}
		}

		socket.getSocket().listen(socket.getNotifier(), basic::server::Socket::createMessageHandler);
	}

	logger.debug << "Start basic servers.\n";
	for(auto& entry : basicServerById) {
		basic::server::Socket& socket = entry.second.get();

		logger.debug << "-> Start basic server \"" << entry.first << "\" for queues:\n";
		if(logger.debug) {
			std::set<std::string> notifiers = socket.getNotifier();
			for(const auto& notifier : notifiers) {
				logger.debug << "   - \"" << notifier << "\"\n";
			}
		}

		socket.getSocket().listen(socket.getNotifier(), basic::server::Socket::createMessageHandler);
	}

	logger.debug << "Start http servers.\n";
	for(auto& entry : httpServerById) {
		http::server::Socket& socket = entry.second.get();

		logger.debug << "-> Start http/https server \"" << entry.first << "\".\n";

		socket.getSocket().listen(http::server::Socket::createRequestHandler);
	}

	bool rc = true;

	if(isDaemon) {
		logger.debug << "Switch into daemon mode.\n";
		if(daemon(1, 1) != 0) {
			rc = false;
			logger.error << "Daemon failed: \"" << strerror(errno) << "\"\n";
		}
	}

	if(rc) {
		rc = messageTimer.run();
	}

	esl::system::SignalHandler::remove(esl::system::SignalHandler::SignalType::pipe, stopFunction);
	esl::system::SignalHandler::remove(esl::system::SignalHandler::SignalType::terminate, stopFunction);
	esl::system::SignalHandler::remove(esl::system::SignalHandler::SignalType::interrupt, stopFunction);

	logger.debug << "Stop basic brokers.\n";
	for(auto& entry : basicBrokerById) {
		basic::server::Socket& socket = entry.second.get().getServer();

		logger.debug << "-> Stop basic broker \"" << entry.first << "\".\n";
		socket.getSocket().release();
	}

	logger.debug << "Stop basic servers.\n";
	for(auto& entry : basicServerById) {
		basic::server::Socket& socket = entry.second.get();

		logger.debug << "-> Stop basic server \"" << entry.first << "\".\n";
		socket.getSocket().release();
	}

	logger.debug << "Stop http servers.\n";
	for(auto& entry : httpServerById) {
		http::server::Socket& socket = entry.second.get();

		logger.debug << "-> Stop http server \"" << socket.getId() << "\".\n";
		socket.getSocket().release();
	}

	logger.debug << "Wait for finished basic brokers.\n";
	for(auto& entry : basicBrokerById) {
		basic::server::Socket& socket = entry.second.get().getServer();

		logger.debug << "-> Waiting for basic broker \"" << entry.first << "\"\n";
		socket.getSocket().wait(0);
		logger.debug << "-> Waiting done.\n";
	}

	logger.debug << "Wait for finished basic servers.\n";
	for(auto& entry : basicServerById) {
		basic::server::Socket& socket = entry.second.get();

		logger.debug << "-> Waiting for basic server \"" << entry.first << "\"\n";
		socket.getSocket().wait(0);
		logger.debug << "-> Waiting done.\n";
	}

	logger.debug << "Wait for finished http servers.\n";
	for(auto& entry : httpServerById) {
		http::server::Socket& socket = entry.second.get();

		logger.debug << "-> Waiting for http server \"" << entry.first << "\"\n";
		socket.getSocket().wait(0);
		logger.debug << "-> Waiting done.\n";
	}

	return rc;
}

bool Engine::runCGI() {
	return true;
}

void Engine::stop() {
	logger.info << "stopping engine\n";
	messageTimer.stop();
}

void Engine::addCertificate(const std::string& hostname, std::vector<unsigned char> key, std::vector<unsigned char> certificate) {
	std::pair<std::vector<unsigned char>, std::vector<unsigned char>>& certPair = certsByHostname[hostname];
	certPair.first = std::move(certificate);
	certPair.second = std::move(key);
}

void Engine::addCertificate(const std::string& hostname, const std::string& keyFile, const std::string& certificateFile) {
    std::ifstream keyIFStream(keyFile, std::ios::binary );
    std::ifstream certificateIFStream(certificateFile, std::ios::binary );

    // copies all data into buffer
    auto key = std::vector<unsigned char>(std::istreambuf_iterator<char>(keyIFStream), {});
    auto certificate = std::vector<unsigned char>(std::istreambuf_iterator<char>(certificateIFStream), {});

//    logger.info << "Key size: " << key.size() << "\n";
//    logger.info << "Certificate size: " << certificate.size() << "\n";

    addCertificate(hostname, std::move(key), std::move(certificate));
}

void Engine::addBasicBroker(const std::string& id, const std::vector<std::pair<std::string, std::string>>& settings, const std::string& implementation) {
	logger.trace << "Adding basic broker (implementation=\"" << implementation << "\") with id=\"" << id << "\"\n";

	if(basicBrokerById.count(id) != 0) {
        throw std::runtime_error("There is already a basic brokers defined with id \"" + id + "\".");
	}

	if(basicServerById.count(id) != 0) {
        throw std::runtime_error("There is already a basic servers defined with id \"" + id + "\".");
	}

	if(httpServerById.count(id) != 0) {
        throw std::runtime_error("There is already a http servers defined with id \"" + id + "\".");
	}

	esl::object::ObjectContext* thisObjectContext = this;
	if(thisObjectContext->findObject<esl::object::Interface::Object>(id)) {
	//if(findObject<esl::object::Interface::Object>(id)) {
        throw std::runtime_error("There is already an object defined with id \"" + id + "\".");
	}

	basic::broker::Client* basicBrokerPtr = new basic::broker::Client(*this, id, settings, implementation);
	std::unique_ptr<esl::object::Interface::Object> object(basicBrokerPtr);
	addObject(id, std::move(object));

	basicBrokerById.insert(std::make_pair(id, std::ref(*basicBrokerPtr)));
}

void Engine::addBasicServer(const std::string& id, const std::vector<std::pair<std::string, std::string>>& settings, const std::string& implementation) {
	logger.trace << "Adding basic server (implementation=\"" << implementation << "\") with id=\"" << id << "\"\n";

	if(basicBrokerById.count(id) != 0) {
        throw std::runtime_error("There is already a basic brokers defined with id \"" + id + "\".");
	}

	if(basicServerById.count(id) != 0) {
        throw std::runtime_error("There is already a basic servers defined with id \"" + id + "\".");
	}

	if(httpServerById.count(id) != 0) {
        throw std::runtime_error("There is already a http servers defined with id \"" + id + "\".");
	}

	esl::object::ObjectContext* thisObjectContext = this;
	if(thisObjectContext->findObject<esl::object::Interface::Object>(id)) {
	//if(findObject<esl::object::Interface::Object>(id)) {
        throw std::runtime_error("There is already an object defined with id \"" + id + "\".");
	}

	basic::server::Socket* basicSocketPtr = new basic::server::Socket(*this, id, settings, implementation);
	std::unique_ptr<esl::object::Interface::Object> object(basicSocketPtr);
	addObject(id, std::move(object));

	basicServerById.insert(std::make_pair(id, std::ref(*basicSocketPtr)));
}

void Engine::addHttpServer(const std::string& id, bool isHttps, const std::vector<std::pair<std::string, std::string>>& settings, const std::string& implementation) {
	if(isHttps) {
		logger.trace << "Adding HTTPS server (implementation=\"" << implementation << "\") with id=\"" << id << "\"\n";
	}
	else {
		logger.trace << "Adding HTTP server (implementation=\"" << implementation << "\") with id=\"" << id << "\"\n";
	}

	if(basicBrokerById.count(id) != 0) {
        throw std::runtime_error("There is already a basic brokers defined with id \"" + id + "\".");
	}

	if(basicServerById.count(id) != 0) {
        throw std::runtime_error("There is already a basic servers defined with id \"" + id + "\".");
	}

	if(httpServerById.count(id) != 0) {
        throw std::runtime_error("There is already a http servers defined with id \"" + id + "\".");
	}

	esl::object::ObjectContext* thisObjectContext = this;
	if(thisObjectContext->findObject<esl::object::Interface::Object>(id)) {
	//if(findObject<esl::object::Interface::Object>(id)) {
        throw std::runtime_error("There is already an object defined with id \"" + id + "\".");
	}

	http::server::Socket* httpSocketPtr = new http::server::Socket(*this, id, isHttps, settings, implementation);
	std::unique_ptr<esl::object::Interface::Object> object(httpSocketPtr);
	addObject(id, std::move(object));

	httpServerById.insert(std::make_pair(id, std::ref(*httpSocketPtr)));
}

basic::server::Listener& Engine::addBasicListener(const std::string& refId, bool inheritObjects) {
	std::unique_ptr<basic::server::Listener> listenerPtr;
	{
		std::vector<std::string> refIds = esl::utility::String::split(refId, ',');
		for(auto& refId : refIds) {
			refId = esl::utility::String::trim(refId);
		}

		listenerPtr.reset(new basic::server::Listener(*this, inheritObjects, std::move(refIds)));
	}
	basic::server::Listener& listener = *listenerPtr;

	for(const auto& refId : listener.getRefIds()) {
		esl::object::ObjectContext* thisObjectContext = this;
		esl::object::Interface::Object* object = thisObjectContext->findObject<esl::object::Interface::Object>(refId);
		//esl::object::Interface::Object* object = findObject<esl::object::Interface::Object>(refId);
		if(!object) {
	        throw std::runtime_error("basic-listener is referencing an unknown object with id \"" + refId + "\".");
		}

		basic::server::Socket* basicServer = nullptr;
		auto basicBroker = dynamic_cast<basic::broker::Client*>(object);
		if(basicBroker) {
			basicServer = &basicBroker->getServer();
		}
		else {
			basicServer = dynamic_cast<basic::server::Socket*>(object);
		}

		if(!basicServer) {
	        throw std::runtime_error("basic-listener is referencing an object with id \"" + refId + "\", but object is neither a basic-broker nor a basic-server.");
		}

		basicServer->addListener(listener);
	}

	basicListeners.push_back(std::move(listenerPtr));

	return listener;
}

http::server::Listener& Engine::addHttpListener(const std::string& refId, bool inheritObjects, const std::string& hostname) {
	std::unique_ptr<http::server::Listener> listenerPtr;
	{
		std::vector<std::string> refIds = esl::utility::String::split(refId, ',');
		for(auto& refId : refIds) {
			refId = esl::utility::String::trim(refId);
		}

		listenerPtr.reset(new http::server::Listener(*this, inheritObjects, hostname, std::move(refIds)));
	}
	http::server::Listener& listener = *listenerPtr;

	for(const auto& refId : listener.getRefIds()) {
		esl::object::ObjectContext* thisObjectContext = this;
		esl::object::Interface::Object* object = thisObjectContext->findObject<esl::object::Interface::Object>(refId);
		//esl::object::Interface::Object* object = findObject<esl::object::Interface::Object>(refId);
		if(!object) {
	        throw std::runtime_error("http-listener is referencing an unknown object with id \"" + refId + "\".");
		}

		auto httpServer = dynamic_cast<http::server::Socket*>(object);
		if(!httpServer) {
	        throw std::runtime_error("http-listener is referencing an object with id \"" + refId + "\", but object is not a http-server.");
		}

		httpServer->addListener(listener);
	}

	httpListeners.push_back(std::move(listenerPtr));

	return listener;
}

void Engine::dumpTree(std::size_t depth) const {
	for(std::size_t i=0; i<depth; ++i) {
		logger.info << "|   ";
	}
	logger.info << "+-> Engine\n";
	++depth;

	BaseContext::dumpTree(depth);

	dumpTreeBasicBrokers(depth);
	dumpTreeBasicServers(depth);
	dumpTreeHttpServers(depth);
	dumpTreeBasicListener(depth);
	dumpTreeHttpListener(depth);
}

void Engine::dumpTreeBasicBrokers(std::size_t depth) const {
	for(std::size_t i=0; i<depth; ++i) {
		logger.info << "|   ";
	}
	logger.info << "+-> Basic brokers\n";
	++depth;

	for(const auto& entry: basicBrokerById) {
		entry.second.get().dumpTree(depth);
	}
}

void Engine::dumpTreeBasicServers(std::size_t depth) const {
	for(std::size_t i=0; i<depth; ++i) {
		logger.info << "|   ";
	}
	logger.info << "+-> Basic servers\n";
	++depth;

	for(const auto& entry: basicServerById) {
		entry.second.get().dumpTree(depth);
	}
}

void Engine::dumpTreeHttpServers(std::size_t depth) const {
	for(std::size_t i=0; i<depth; ++i) {
		logger.info << "|   ";
	}
	logger.info << "+-> HTTP servers\n";
	++depth;

	for(const auto& entry: httpServerById) {
		entry.second.get().dumpTree(depth);
	}
}

void Engine::dumpTreeBasicListener(std::size_t depth) const {
	for(std::size_t i=0; i<depth; ++i) {
		logger.info << "|   ";
	}
	logger.info << "+-> Basic listeners\n";
	++depth;

	for(const auto& listener: basicListeners) {
		listener->dumpTree(depth);
	}
}

void Engine::dumpTreeHttpListener(std::size_t depth) const {
	for(std::size_t i=0; i<depth; ++i) {
		logger.info << "|   ";
	}
	logger.info << "+-> HTTP listeners\n";
	++depth;

	for(const auto& listener: httpListeners) {
		listener->dumpTree(depth);
	}
}

} /* namespace engine */
} /* namespace jerry */
