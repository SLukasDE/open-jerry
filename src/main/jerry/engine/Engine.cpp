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
#include <jerry/engine/ObjectContext.h>
#include <jerry/Logger.h>

#include <esl/Module.h>
#include <esl/object/InitializeContext.h>
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

	/* ************************* *
	 * initialize global objects *
	 * ************************* */
	logger.info << "Initialize context of global objects, brokers, servers and context ...\n";
	initializeContext();
	logger.info << "Initialization of global objects, brokers, servers and context done.\n";

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
	for(const auto& entry: getObjects()) {
		http::server::Socket* httpServerPtr = dynamic_cast<http::server::Socket*>(&entry.second.get());

		/* add certificates to http-socket if https is used */
		if(httpServerPtr && httpServerPtr->isHttps()) {
			/* host names for that we need a certificate */
			std::set<std::string> hostnames = httpServerPtr->getHostnames();

			/* add certificate http-socket for each host name */
			for(const auto& hostname : hostnames) {
				auto certIter = certsByHostname.find(hostname);
				if(certIter == std::end(certsByHostname)) {
					throw std::runtime_error("No certificate available for hostname '" + hostname + "'.");
				}
				httpServerPtr->addTLSHost(hostname, certIter->second.first, certIter->second.second);
			}
		}
	}

	auto stopFunction = [this]() { stop(); };
	esl::system::SignalHandler::install(esl::system::SignalHandler::SignalType::interrupt, stopFunction);
	esl::system::SignalHandler::install(esl::system::SignalHandler::SignalType::terminate, stopFunction);
	esl::system::SignalHandler::install(esl::system::SignalHandler::SignalType::pipe, stopFunction);

	logger.debug << "\n";
	logger.debug << "Starting daemons and servers ...\n";
	logger.debug << "---------------------------------------------------\n";
	for(const auto& entry: getObjects()) {
		esl::object::Interface::Object* objectPtr = &entry.second.get();
		basic::server::Socket* basicServerPtr = dynamic_cast<basic::server::Socket*>(objectPtr);
		http::server::Socket* httpServerPtr = dynamic_cast<http::server::Socket*>(objectPtr);

		// Check if object is basic::server::Socket
		if(basicServerPtr) {
			logger.debug << "-> Start basic server \"" << entry.first << "\" for queues:\n";
			if(logger.debug) {
				std::set<std::string> notifiers = basicServerPtr->getNotifiers();
				for(const auto& notifier : notifiers) {
					logger.debug << "   - \"" << notifier << "\"\n";
				}
			}

			basicServerPtr->listen([]{});
			logger.debug << "   ... done.\n";
		}

		// Check if object is http::server::Socket
		if(httpServerPtr) {
			logger.debug << "-> Start http/https server \"" << entry.first << "\" ...\n";
			httpServerPtr->listen([]{});
			logger.debug << "   ... done.\n";
		}
	}
	logger.debug << "---------------------------------------------------\n";
	logger.debug << "All daemons and servers have been started.\n";

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

	logger.debug << "\n";
	logger.debug << "Stopping brokers, daemons and servers ...\n";
	logger.debug << "---------------------------------------------------\n";
	for(const auto& entry: getObjects()) {
		esl::object::Interface::Object* objectPtr = &entry.second.get();
		basic::server::Socket* basicServerPtr = dynamic_cast<basic::server::Socket*>(objectPtr);
		http::server::Socket* httpServerPtr = dynamic_cast<http::server::Socket*>(objectPtr);

		// Check if object is basic::server::Socket
		if(basicServerPtr) {
			logger.debug << "-> Stopping basic server \"" << entry.first << "\".\n";
			basicServerPtr->release();
		}

		// Check if object is http::server::Socket
		if(httpServerPtr) {
			logger.debug << "-> Stopping http server \"" << entry.first << "\".\n";
			httpServerPtr->release();
		}
	}

	logger.debug << "---------------------------------------------------\n";
	logger.debug << "Stopping brokers, daemons and servers initiated.\n";
	logger.debug << "\n";
	logger.debug << "Wait for stopped brokers, daemons and servers ...\n";
	logger.debug << "---------------------------------------------------\n";
	for(const auto& entry: getObjects()) {
		esl::object::Interface::Object* objectPtr = &entry.second.get();
		basic::server::Socket* basicServerPtr = dynamic_cast<basic::server::Socket*>(objectPtr);
		http::server::Socket* httpServerPtr = dynamic_cast<http::server::Socket*>(objectPtr);

		// Check if object is basic::server::Socket
		if(basicServerPtr) {
			logger.debug << "-> Waiting for basic server \"" << entry.first << "\" ...\n";
			basicServerPtr->wait(0);
			logger.debug << "   ... done.\n";
		}

		// Check if object is http::server::Socket
		if(httpServerPtr) {
			logger.debug << "-> Waiting for http server \"" << entry.first << "\" ...\n";
			httpServerPtr->wait(0);
			logger.debug << "   ... done.\n";
		}
	}
	logger.debug << "---------------------------------------------------\n";
	logger.debug << "All brokers, daemons and servers have been stopped.\n";

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

void Engine::addBasicServer(const std::string& id, const std::vector<std::pair<std::string, std::string>>& settings, const std::string& implementation) {
	logger.trace << "Adding basic server (implementation=\"" << implementation << "\") with id=\"" << id << "\"\n";
	addObject(id, std::unique_ptr<esl::object::Interface::Object>(new basic::server::Socket(id, settings, implementation)));
}

void Engine::addHttpServer(const std::string& id, bool isHttps, const std::vector<std::pair<std::string, std::string>>& settings, const std::string& implementation) {
	if(isHttps) {
		logger.trace << "Adding HTTPS server (implementation=\"" << implementation << "\") with id=\"" << id << "\"\n";
	}
	else {
		logger.trace << "Adding HTTP server (implementation=\"" << implementation << "\") with id=\"" << id << "\"\n";
	}

	addObject(id, std::unique_ptr<esl::object::Interface::Object>(new http::server::Socket(id, isHttps, settings, implementation)));
}

basic::server::Listener& Engine::addBasicListener(const std::string& refId, bool inheritObjects) {
	std::unique_ptr<basic::server::Listener> basicListener;
	{
		std::vector<std::string> refIds = esl::utility::String::split(refId, ',');
		for(auto& refId : refIds) {
			refId = esl::utility::String::trim(refId);
		}

		basicListener.reset(new basic::server::Listener(std::move(refIds)));
		if(inheritObjects) {
			basicListener->setParent(this);
		}
	}

	for(const auto& refId : basicListener->getRefIds()) {
		basic::server::Socket* basicServer = findObject<basic::server::Socket>(refId);
		if(!basicServer) {
	        throw std::runtime_error("basic-listener is referencing an object with id \"" + refId + "\", but no basic-server or basic-broker has been found with this id.");
		}

		basicServer->addListener(*basicListener);
	}

	basic::server::Listener& rv = *basicListener;
	basicListeners.push_back(std::move(basicListener));

	return rv;
}

http::server::Listener& Engine::addHttpListener(const std::string& refId, bool inheritObjects, const std::string& hostname) {
	std::unique_ptr<http::server::Listener> httpListener;
	{
		std::vector<std::string> refIds = esl::utility::String::split(refId, ',');
		for(auto& refId : refIds) {
			refId = esl::utility::String::trim(refId);
		}

		httpListener.reset(new http::server::Listener(hostname, std::move(refIds)));
		if(inheritObjects) {
			jerry::engine::ObjectContext* objectContext = httpListener.get();
			objectContext->setParent(this);
		}
	}

	for(const auto& refId : httpListener->getRefIds()) {
		auto httpServer = findObject<http::server::Socket>(refId);
		if(!httpServer) {
	        throw std::runtime_error("http-listener is referencing an object with id \"" + refId + "\", but no http-server has been found with this id.");
		}
		httpServer->addListener(*httpListener);
	}

	http::server::Listener& rv = *httpListener;
	httpListeners.push_back(std::move(httpListener));

	return rv;
}

void Engine::dumpTree(std::size_t depth) const {
	for(std::size_t i=0; i<depth; ++i) {
		logger.info << "|   ";
	}
	logger.info << "+-> Engine\n";
	++depth;

	dumpTreeEntries(depth);
	dumpTreeBasicListener(depth);
	dumpTreeHttpListener(depth);
}

void Engine::dumpTreeEntries(std::size_t depth) const {
	for(const auto& entry: getObjects()) {
		const esl::object::Interface::Object* objectPtr = &entry.second.get();
		const basic::server::Socket* basicServerPtr = dynamic_cast<const basic::server::Socket*>(objectPtr);
		const basic::server::Context* basicContextPtr = dynamic_cast<const basic::server::Context*>(objectPtr);

		const http::server::Socket* httpServerPtr = dynamic_cast<const http::server::Socket*>(objectPtr);
		const http::server::Context* httpContextPtr = dynamic_cast<const http::server::Context*>(objectPtr);

		if(basicServerPtr) {
			for(std::size_t i=0; i<depth; ++i) {
				logger.info << "|   ";
			}
			logger.info << "+-> Basic server: \"" << entry.first << "\" -> " << basicServerPtr << "\n";
			basicServerPtr->dumpTree(depth+1);
		}

		else if(httpServerPtr) {
			for(std::size_t i=0; i<depth; ++i) {
				logger.info << "|   ";
			}
			logger.info << "+-> HTTP server: \"" << entry.first << "\" -> " << httpServerPtr << "\n";
			httpServerPtr->dumpTree(depth+1);
		}

		else if(basicContextPtr) {
			for(std::size_t i=0; i<depth; ++i) {
				logger.info << "|   ";
			}
			logger.info << "+-> Basic-Context: \"" << entry.first << "\" -> " << basicContextPtr << "\n";
			basicContextPtr->dumpTree(depth+1);
		}

		else if(httpContextPtr) {
			for(std::size_t i=0; i<depth; ++i) {
				logger.info << "|   ";
			}
			logger.info << "+-> HTTP-Context: \"" << entry.first << "\" -> " << httpContextPtr << "\n";
			httpContextPtr->dumpTree(depth+1);
		}

		else if(objectPtr) {
			for(std::size_t i=0; i<depth; ++i) {
				logger.info << "|   ";
			}
			logger.info << "+-> Object: \"" << entry.first << "\" -> " << objectPtr << "\n";
		}
	}
}

void Engine::dumpTreeBasicListener(std::size_t depth) const {
	for(const auto& listener: basicListeners) {
		for(std::size_t i=0; i<depth; ++i) {
			logger.info << "|   ";
		}
		logger.info << "+-> Basic listener: -> " << listener.get() << "\n";
		listener->dumpTree(depth + 1);
	}
}

void Engine::dumpTreeHttpListener(std::size_t depth) const {
	for(const auto& listener: httpListeners) {
		for(std::size_t i=0; i<depth; ++i) {
			logger.info << "|   ";
		}
		logger.info << "+-> HTTP listener: -> " << listener.get() << "\n";

		listener->dumpTree(depth + 1);
	}
}

} /* namespace engine */
} /* namespace jerry */
