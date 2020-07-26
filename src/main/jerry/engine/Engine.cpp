/*
 * This file is part of Jerry application server.
 * Copyright (C) 2020 Sven Lukas
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
#include <jerry/engine/RequestHandler.h>
#include <jerry/engine/Endpoint.h>
#include <jerry/engine/ExceptionHandler.h>
#include <jerry/Logger.h>
#include <jerry/Module.h>

#include <esl/Module.h>
#include <esl/Stacktrace.h>
#include <esl/system/SignalHandler.h>
#include <esl/http/server/Request.h>
#include <esl/http/server/RequestContext.h>
#include <esl/http/server/exception/StatusCode.h>
#include <esl/http/server/requesthandler/Interface.h>
#include <esl/utility/String.h>

#include <set>
#include <fstream>
#include <vector>
#include <stdexcept>
#include <algorithm>

namespace jerry {
namespace engine {

namespace {
Logger logger("jerry::engine::Engine");
} /* anonymous namespace */

Engine::Engine()
: engineObject(*this),
  messageTimer([this](const Message& message) { message.f(*this); })
{ }

bool Engine::run() {
	runThreadId = std::this_thread::get_id();
	if(listenerByPort.empty()) {
		logger.warn << "no listener defined.\n";
		return false;
	}

	/* *********************************************************** *
	 * initialize ExceptionHandler:                                *
	 * Load all implementations to convert 'const std::exception&' *
	 * to esl::http::server::exception::Interface::Message         *
	 * *********************************************************** */
	ExceptionHandler::initialize();

	/* ********************************** *
	 * initialize objects for all context *
	 * ********************************** */
	logger.info << "Initialize context of global objects...\n";
	initializeContext();
	logger.info << "Initialization of global objects done.\n";
	for(auto& portListener : listenerByPort) {
		for(auto& listener : portListener.second.listenerByDomain) {
			logger.info << "Initialize context of objects for listener \"" << portListener.second.protocol << "://" << listener.first << ":" << portListener.first << "\" ...\n";
			listener.second->initializeContext();
			logger.info << "Initialization of objects for listener \"" << portListener.second.protocol << "://" << listener.first << ":" << portListener.first << "\" done.\n";
		}
	}

	/* **************************************************** *
	 * create sockets for all ports and                     *
	 * add certificates to socket if port is used for https *
	 * **************************************************** */
	std::vector<std::unique_ptr<esl::http::server::Socket>> sockets;
	for(const auto& portListener : listenerByPort) {
		std::unique_ptr<esl::http::server::Socket> socket(new esl::http::server::Socket(portListener.first, createRequestHandler, {{"threads", "4"}}));
		socket->setObject("", [this](const esl::http::server::RequestContext&){ return &this->engineObject; });

		bool isHttps = (portListener.second.protocol == "https");

		if(isHttps) {
			for(const auto& aListener : portListener.second.listenerByDomain) {
				auto certIter = certsByHostname.find(aListener.first);
				if(certIter == std::end(certsByHostname)) {
					throw std::runtime_error("No certificate available for hostname '" + aListener.first + "'.");
				}
				socket->addTLSHost(aListener.first, certIter->second.first, certIter->second.second);
			}
		}

		sockets.push_back(std::move(socket));
	}

	auto stopFunction = [this]() { stop(); };
	esl::system::SignalHandler::install(esl::system::SignalHandler::SignalType::interrupt, stopFunction);
	esl::system::SignalHandler::install(esl::system::SignalHandler::SignalType::terminate, stopFunction);
	esl::system::SignalHandler::install(esl::system::SignalHandler::SignalType::pipe, stopFunction);

	for(auto& socket : sockets) {
		socket->listen();
	}
	bool rc = messageTimer.run();

	esl::system::SignalHandler::remove(esl::system::SignalHandler::SignalType::pipe, stopFunction);
	esl::system::SignalHandler::remove(esl::system::SignalHandler::SignalType::terminate, stopFunction);
	esl::system::SignalHandler::remove(esl::system::SignalHandler::SignalType::interrupt, stopFunction);

	for(auto& socket : sockets) {
		socket->release();
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

void Engine::addTLSHost(const std::string& hostname, std::vector<unsigned char> certificate, std::vector<unsigned char> key) {
	std::pair<std::vector<unsigned char>, std::vector<unsigned char>>& certPair = certsByHostname[hostname];
	certPair.first = std::move(certificate);
	certPair.second = std::move(key);
}

void Engine::addCertificate(const std::string& domain, const std::string& keyFile, const std::string& certificateFile) {
    std::ifstream keyIFStream(keyFile, std::ios::binary );
    std::ifstream certificateIFStream(certificateFile, std::ios::binary );

    // copies all data into buffer
    auto key = std::vector<unsigned char>(std::istreambuf_iterator<char>(keyIFStream), {});
    auto certificate = std::vector<unsigned char>(std::istreambuf_iterator<char>(certificateIFStream), {});

//    logger.info << "Key size: " << key.size() << "\n";
//    logger.info << "Certificate size: " << certificate.size() << "\n";

    addTLSHost(domain, std::move(certificate), std::move(key));
}

Listener& Engine::addListener(utility::URL url) {

	uint16_t port = 80;
	std::string scheme;
	std::string hostname;

	if(url.getScheme().empty()) {
		scheme = "http";
	}
	else if(url.getScheme() == "http") {
		scheme = "http";
	}
	else if(url.getScheme() == "https") {
		scheme = "https";
	}
	else {
		throw std::runtime_error("unkown protocol \"" + url.getScheme() + "\"");
	}

	if(!url.getPort().empty()) {
		port = std::stoi(url.getPort());
	}
	else if(scheme == "http") {
		port = 80;
	}
	else if(scheme == "https") {
		port = 443;
	}

	if(url.getHostname().empty()) {
		hostname = "*";
	}
	else {
		hostname = url.getHostname();
	}

	logger.trace << "Adding listener \"" << scheme << "://" << hostname << ":" << port << "\"\n";

	auto iterListenerByPort = listenerByPort.find(port);
	if(iterListenerByPort == std::end(listenerByPort)) {
		iterListenerByPort = listenerByPort.insert(std::make_pair(port, PortListener(scheme))).first;
	}
	else if(iterListenerByPort->second.protocol != scheme) {
        throw std::runtime_error("Wrong protocol: \"" + scheme + "\". Port is already set to protocol \"" + iterListenerByPort->second.protocol + "\".");
	}

	auto iterListenerByDomain = iterListenerByPort->second.listenerByDomain.find(hostname);
	if(iterListenerByDomain != std::end(iterListenerByPort->second.listenerByDomain)) {
        throw std::runtime_error("Wrong domain: \"" + hostname + "\". Domain is already specified to protocol \"" + iterListenerByPort->second.protocol + "\" and port \"" + std::to_string(iterListenerByPort->first) + "\".");
	}

	Listener* newListener = new Listener(*this);
	iterListenerByPort->second.listenerByDomain[hostname] = std::unique_ptr<Listener>(newListener);
	return *newListener;
}

esl::object::Interface::Object* Engine::getObject(const std::string& id) const {
	esl::object::Interface::Object* object = BaseContext::getObject(id);

	if(object == nullptr) {
		logger.warn << "Lookup for object \"" << id << "\" in engine failed.\n";
	}

	return object;
}

std::unique_ptr<esl::http::server::requesthandler::Interface::RequestHandler> Engine::createRequestHandler(esl::http::server::RequestContext& requestContext) {
	/* Access log */
	logger.info << "Request " << requestContext.getRequest().getMethod() << " \"" << requestContext.getRequest().getPath() << "\" received from " << requestContext.getRequest().getRemoteAddress() << "\n";

	EngineObject* engineObject = dynamic_cast<EngineObject*>(requestContext.getObject(""));
	if(!engineObject) {
    	esl::http::server::exception::StatusCode e(500, "Engine object not found");
		ExceptionHandler exceptionHandler;

    	exceptionHandler.setShowException(false);
    	exceptionHandler.setShowStacktrace(false);
    	exceptionHandler.setMessage(e);
    	exceptionHandler.dump(logger.error);
    	exceptionHandler.dump(requestContext.getConnection());
    	return std::unique_ptr<esl::http::server::requesthandler::Interface::RequestHandler>(new esl::http::server::requesthandler::Interface::RequestHandler);
    	/*
		logger.error << "Engine object not found\n";

		return nullptr;
		*/
	}

	Engine& engine = engineObject->getEngine();

	Listener* listener = engine.getListener(requestContext);

	if(listener == nullptr) {
		/* Error log */
		logger.info << "ERROR: No listener available for invalid request \"" << requestContext.getRequest().getPath() << "\"\n";

    	esl::http::server::exception::StatusCode e(404);
		ExceptionHandler exceptionHandler;

    	exceptionHandler.setShowException(false);
    	exceptionHandler.setShowStacktrace(false);
    	exceptionHandler.setMessage(esl::http::server::exception::StatusCode(500));
    	exceptionHandler.dump(requestContext.getConnection());
    	return std::unique_ptr<esl::http::server::requesthandler::Interface::RequestHandler>(new esl::http::server::requesthandler::Interface::RequestHandler);
//		return nullptr;
	}
	return listener->createRequestHandler(requestContext);
}

Listener* Engine::getListener(esl::http::server::RequestContext& requestContext) const {
	auto iterListenerByPort = listenerByPort.find(requestContext.getRequest().getHostPort());
	if(iterListenerByPort == std::end(listenerByPort)) {
		return nullptr;
	}

	std::string domain = requestContext.getRequest().getHost();
	auto iterListenerByDomain = iterListenerByPort->second.listenerByDomain.find(domain);
	while(iterListenerByDomain == std::end(iterListenerByPort->second.listenerByDomain)) {
		std::string::size_type pos = domain.find_first_of('.');
		if(pos == std::string::npos) {
			iterListenerByDomain = iterListenerByPort->second.listenerByDomain.find("*");
			break;
		}
		domain = domain.substr(pos+1);
		iterListenerByDomain = iterListenerByPort->second.listenerByDomain.find("*." + domain);
	}
	if(iterListenerByDomain == std::end(iterListenerByPort->second.listenerByDomain)) {
		return nullptr;
	}

	return iterListenerByDomain->second.get();
}

} /* namespace engine */
} /* namespace jerry */
