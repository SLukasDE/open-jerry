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
#include <jerry/http/RequestHandler.h>
#include <jerry/engine/Endpoint.h>
#include <jerry/Logger.h>
#include <jerry/Module.h>
#include <jerry/URL.h>

#include <esl/Module.h>
#include <esl/Stacktrace.h>
#include <esl/system/SignalHandler.h>
#include <esl/http/server/Request.h>
#include <esl/http/server/RequestContext.h>
#include <esl/utility/String.h>

#include <esl/http/server/handler/Interface.h>

#include <set>
#include <fstream>
#include <vector>
#include <stdexcept>
#include <algorithm>

#include <iostream>

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

    std::vector<std::unique_ptr<esl::http::server::Socket>> sockets;
    for(const auto& portListener : listenerByPort) {
		std::unique_ptr<esl::http::server::Socket> socket(new esl::http::server::Socket(portListener.first, 4, requestHandlerFactory));
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
    esl::system::signalHandlerInstall(esl::system::SignalType::interrupt, stopFunction);
    esl::system::signalHandlerInstall(esl::system::SignalType::terminate, stopFunction);
    esl::system::signalHandlerInstall(esl::system::SignalType::pipe, stopFunction);

	for(auto& socket : sockets) {
		socket->listen();
	}
    bool rc = messageTimer.run();

    esl::system::signalHandlerRemove(esl::system::SignalType::pipe, stopFunction);
    esl::system::signalHandlerRemove(esl::system::SignalType::terminate, stopFunction);
    esl::system::signalHandlerRemove(esl::system::SignalType::interrupt, stopFunction);

	for(auto& socket : sockets) {
		socket->release();
	}

	return rc;
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

Listener& Engine::addListener(URL url) {

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

std::unique_ptr<esl::http::server::RequestHandler> Engine::requestHandlerFactory(esl::http::server::RequestContext& requestContext) {
	EngineObject* engineObject = dynamic_cast<EngineObject*>(requestContext.getObject(""));
	if(!engineObject) {
		return nullptr;
	}

	Engine& engine = engineObject->getEngine();

	Listener* listener = engine.getListener(requestContext);

	if(listener == nullptr) {
		return nullptr;
	}
	return listener->createRequestHandler(requestContext);
}

Listener* Engine::getListener(esl::http::server::RequestContext& requestContext) const {
	auto iterListenerByPort = listenerByPort.find(requestContext.getRequest().getPort());
	if(iterListenerByPort == std::end(listenerByPort)) {
		return nullptr;
	}

	std::string domain = requestContext.getRequest().getDomain();
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
