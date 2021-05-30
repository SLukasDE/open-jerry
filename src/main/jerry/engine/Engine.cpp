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
#include <jerry/Module.h>

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

bool Engine::run() {
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
	 * initialize message-listener objects *
	 * *********************************** */
	logger.info << "Initialize context of all message-listeners ...\n";
	for(auto& listener : messageListeners) {
		if(listener) {
			listener->initializeContext();
		}
	}
	logger.info << "Initialization of all message-listeners done.\n";

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

	logger.debug << "Start message brokers.\n";
	for(auto& entry : messageBrokerById) {
		messaging::server::Socket& messageServer = entry.second.get().getServer();

		logger.debug << "-> Start message broker \"" << entry.first << "\" for queues:\n";
		if(logger.debug) {
			std::set<std::string> notifiers = messageServer.getNotifier();
			for(const auto& notifier : notifiers) {
				logger.debug << "   - \"" << notifier << "\"\n";
			}
		}

		messageServer.getSocket().listen(messageServer.getNotifier(), messaging::server::Socket::createMessageHandler);
	}

	logger.debug << "Start message servers.\n";
	for(auto& entry : messageServerById) {
		messaging::server::Socket& messageServer = entry.second.get();

		logger.debug << "-> Start message server \"" << entry.first << "\" for queues:\n";
		if(logger.debug) {
			std::set<std::string> notifiers = messageServer.getNotifier();
			for(const auto& notifier : notifiers) {
				logger.debug << "   - \"" << notifier << "\"\n";
			}
		}

		messageServer.getSocket().listen(messageServer.getNotifier(), messaging::server::Socket::createMessageHandler);
	}

	logger.debug << "Start http servers.\n";
	for(auto& entry : httpServerById) {
		http::server::Socket& httpServer = entry.second.get();

		logger.debug << "-> Start http/https server \"" << entry.first << "\".\n";

		httpServer.getSocket().listen(http::server::Socket::createRequestHandler);
	}

	bool rc = messageTimer.run();

	esl::system::SignalHandler::remove(esl::system::SignalHandler::SignalType::pipe, stopFunction);
	esl::system::SignalHandler::remove(esl::system::SignalHandler::SignalType::terminate, stopFunction);
	esl::system::SignalHandler::remove(esl::system::SignalHandler::SignalType::interrupt, stopFunction);

	logger.debug << "Stop message brokers.\n";
	for(auto& entry : messageBrokerById) {
		messaging::server::Socket& messageServer = entry.second.get().getServer();

		logger.debug << "-> Stop message broker \"" << entry.first << "\".\n";
		messageServer.getSocket().release();
	}

	logger.debug << "Stop message servers.\n";
	for(auto& entry : messageServerById) {
		messaging::server::Socket& messageServer = entry.second.get();

		logger.debug << "-> Stop message server \"" << entry.first << "\".\n";
		messageServer.getSocket().release();
	}

	logger.debug << "Stop http servers.\n";
	for(auto& entry : httpServerById) {
		http::server::Socket& httpServer = entry.second.get();

		logger.debug << "-> Stop http server \"" << httpServer.getId() << "\".\n";
		httpServer.getSocket().release();
	}

	logger.debug << "Wait for finished message broker.\n";
	for(auto& entry : messageBrokerById) {
		messaging::server::Socket& messageServer = entry.second.get().getServer();

		logger.debug << "-> Waiting for message broker \"" << entry.first << "\"\n";
		messageServer.getSocket().wait(0);
		logger.debug << "-> Waiting done.\n";
	}

	logger.debug << "Wait for finished message servers.\n";
	for(auto& entry : messageServerById) {
		messaging::server::Socket& messageServer = entry.second.get();

		logger.debug << "-> Waiting for message server \"" << entry.first << "\"\n";
		messageServer.getSocket().wait(0);
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

void Engine::addMessageBroker(const std::string& id, const std::string& brokers, const std::vector<std::pair<std::string, std::string>>& settings, const std::string& implementation) {
	logger.trace << "Adding message broker (implementation=\"" << implementation << "\") with id=\"" << id << "\" for broker " << brokers << "\"\n";

	if(messageBrokerById.count(id) != 0) {
        throw std::runtime_error("There is already a message brokers defined with id \"" + id + "\".");
	}

	if(messageServerById.count(id) != 0) {
        throw std::runtime_error("There is already a message servers defined with id \"" + id + "\".");
	}

	if(httpServerById.count(id) != 0) {
        throw std::runtime_error("There is already a http servers defined with id \"" + id + "\".");
	}

	esl::object::ObjectContext* thisObjectContext = this;
	if(thisObjectContext->findObject<esl::object::Interface::Object>(id)) {
	//if(findObject<esl::object::Interface::Object>(id)) {
        throw std::runtime_error("There is already an object defined with id \"" + id + "\".");
	}

	messaging::broker::Client* messageBrokerPtr = new messaging::broker::Client(*this, id, brokers, settings, implementation);
	std::unique_ptr<esl::object::Interface::Object> object(messageBrokerPtr);
	addObject(id, std::move(object));

	messageBrokerById.insert(std::make_pair(id, std::ref(*messageBrokerPtr)));
}

void Engine::addMessageServer(const std::string& id, std::uint16_t port, const std::vector<std::pair<std::string, std::string>>& settings, const std::string& implementation) {
	logger.trace << "Adding message server (implementation=\"" << implementation << "\") with id=\"" << id << "\" at port " << port << "\"\n";

	if(listeningPorts.count(port) > 0) {
        throw std::runtime_error("There are multiple servers listening for port " + std::to_string(port) + ".");
	}

	if(messageBrokerById.count(id) != 0) {
        throw std::runtime_error("There is already a message brokers defined with id \"" + id + "\".");
	}

	if(messageServerById.count(id) != 0) {
        throw std::runtime_error("There is already a message servers defined with id \"" + id + "\".");
	}

	if(httpServerById.count(id) != 0) {
        throw std::runtime_error("There is already a http servers defined with id \"" + id + "\".");
	}

	esl::object::ObjectContext* thisObjectContext = this;
	if(thisObjectContext->findObject<esl::object::Interface::Object>(id)) {
	//if(findObject<esl::object::Interface::Object>(id)) {
        throw std::runtime_error("There is already an object defined with id \"" + id + "\".");
	}

	messaging::server::Socket* messageSocketPtr = new messaging::server::Socket(*this, id, port, settings, implementation);
	std::unique_ptr<esl::object::Interface::Object> object(messageSocketPtr);
	addObject(id, std::move(object));

	listeningPorts.insert(port);
	messageServerById.insert(std::make_pair(id, std::ref(*messageSocketPtr)));
}

void Engine::addHttpServer(const std::string& id, std::uint16_t port, bool isHttps, const std::vector<std::pair<std::string, std::string>>& settings, const std::string& implementation) {
	if(isHttps) {
		logger.trace << "Adding HTTPS server (implementation=\"" << implementation << "\") with id=\"" << id << "\" at port " << port << "\"\n";
	}
	else {
		logger.trace << "Adding HTTP server (implementation=\"" << implementation << "\") with id=\"" << id << "\" at port " << port << "\"\n";
	}

	if(listeningPorts.count(port) > 0) {
        throw std::runtime_error("There are multiple servers listening for port " + std::to_string(port) + ".");
	}

	if(messageBrokerById.count(id) != 0) {
        throw std::runtime_error("There is already a message brokers defined with id \"" + id + "\".");
	}

	if(messageServerById.count(id) != 0) {
        throw std::runtime_error("There is already a message servers defined with id \"" + id + "\".");
	}

	if(httpServerById.count(id) != 0) {
        throw std::runtime_error("There is already a http servers defined with id \"" + id + "\".");
	}

	esl::object::ObjectContext* thisObjectContext = this;
	if(thisObjectContext->findObject<esl::object::Interface::Object>(id)) {
	//if(findObject<esl::object::Interface::Object>(id)) {
        throw std::runtime_error("There is already an object defined with id \"" + id + "\".");
	}

	http::server::Socket* httpSocketPtr = new http::server::Socket(*this, id, port, isHttps, settings, implementation);
	std::unique_ptr<esl::object::Interface::Object> object(httpSocketPtr);
	addObject(id, std::move(object));

	listeningPorts.insert(port);
	httpServerById.insert(std::make_pair(id, std::ref(*httpSocketPtr)));
}

messaging::server::Listener& Engine::addMessageListener(const std::string& refId, bool inheritObjects) {
	std::unique_ptr<messaging::server::Listener> listenerPtr;
	{
		std::vector<std::string> refIds = esl::utility::String::split(refId, ',');
		for(auto& refId : refIds) {
			refId = esl::utility::String::trim(refId);
		}

		listenerPtr.reset(new messaging::server::Listener(*this, inheritObjects, std::move(refIds)));
	}
	messaging::server::Listener& listener = *listenerPtr;

	for(const auto& refId : listener.getRefIds()) {
		esl::object::ObjectContext* thisObjectContext = this;
		esl::object::Interface::Object* object = thisObjectContext->findObject<esl::object::Interface::Object>(refId);
		//esl::object::Interface::Object* object = findObject<esl::object::Interface::Object>(refId);
		if(!object) {
	        throw std::runtime_error("message-listener is referencing an unknown object with id \"" + refId + "\".");
		}

		messaging::server::Socket* messageServer = nullptr;
		auto messageBroker = dynamic_cast<messaging::broker::Client*>(object);
		if(messageBroker) {
			messageServer = &messageBroker->getServer();
		}
		else {
			messageServer = dynamic_cast<messaging::server::Socket*>(object);
		}

		if(!messageServer) {
	        throw std::runtime_error("message-listener is referencing an object with id \"" + refId + "\", but object is neither a message-broker nor a message-server.");
		}

		messageServer->addListener(listener);
	}

	messageListeners.push_back(std::move(listenerPtr));

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

	dumpTreeMessageBrokers(depth);
	dumpTreeMessageServers(depth);
	dumpTreeHttpServers(depth);
	dumpTreeMessageListener(depth);
	dumpTreeHttpListener(depth);
}

void Engine::dumpTreeMessageBrokers(std::size_t depth) const {
	for(std::size_t i=0; i<depth; ++i) {
		logger.info << "|   ";
	}
	logger.info << "+-> Messaging brokers\n";
	++depth;

	for(const auto& entry: messageBrokerById) {
		entry.second.get().dumpTree(depth);
	}
}

void Engine::dumpTreeMessageServers(std::size_t depth) const {
	for(std::size_t i=0; i<depth; ++i) {
		logger.info << "|   ";
	}
	logger.info << "+-> Messaging servers\n";
	++depth;

	for(const auto& entry: messageServerById) {
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

void Engine::dumpTreeMessageListener(std::size_t depth) const {
	for(std::size_t i=0; i<depth; ++i) {
		logger.info << "|   ";
	}
	logger.info << "+-> Message listeners\n";
	++depth;

	for(const auto& listener: messageListeners) {
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
