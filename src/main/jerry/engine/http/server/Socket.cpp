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

#include <jerry/engine/http/server/Socket.h>
#include <jerry/engine/http/server/ExceptionHandler.h>
#include <jerry/Logger.h>

#include <esl/object/Properties.h>
#include <esl/object/Interface.h>
#include <esl/Stacktrace.h>

#include <stdexcept>

namespace jerry {
namespace engine {
namespace http {
namespace server {

namespace {
Logger logger("jerry::engine::http::server::Socket");
}

Socket::Socket(Engine& aEngine, const std::string& aId, std::uint16_t aPort, bool aHttps,
		const std::vector<std::pair<std::string, std::string>>& settings,
		const std::string& aImplementation)
: socket(aPort, esl::object::Properties(settings), aImplementation),
  engine(aEngine),
  id(aId),
  port(aPort),
  https(aHttps),
  implementation(aImplementation)
{
	getSocket().addObjectFactory("", [this](const esl::http::server::RequestContext&) {
		esl::object::Interface::Object* object = this;
		return object;
	});
}

void Socket::addTLSHost(const std::string& hostname, std::vector<unsigned char> certificate, std::vector<unsigned char> key) {
	logger.warn << "Calling 'addTLSHost' is not allowed.\n";
	//throw esl::addStacktrace(std::runtime_error("Calling 'addTLSHost' is not allowed."));
}

void Socket::addObjectFactory(const std::string& id, ObjectFactory objectFactory) {
	logger.warn << "Calling 'addObjectFactory' is not allowed.\n";
	//throw esl::addStacktrace(std::runtime_error("Calling 'addObjectFactory' is not allowed."));
}

void Socket::listen(esl::http::server::requesthandler::Interface::CreateInput createInput) {
	logger.warn << "Calling 'listen' is not allowed.\n";
	//throw esl::addStacktrace(std::runtime_error("Calling 'listen' is not allowed."));
}

void Socket::release() {
	logger.warn << "Calling 'release' is not allowed.\n";
	//throw esl::addStacktrace(std::runtime_error("Calling 'release' is not allowed."));
}

void Socket::dumpTree(std::size_t depth) const {
	for(std::size_t i=0; i<depth; ++i) {
		logger.info << "|   ";
	}
	logger.info << "+-> ID: \"" << getId() << "\"\n";
	++depth;

	for(std::size_t i=0; i<depth; ++i) {
		logger.info << "|   ";
	}
	logger.info << "Implementation: \"" << getImplementation() << "\"\n";

	for(std::size_t i=0; i<depth; ++i) {
		logger.info << "|   ";
	}
	logger.info << "Port: " << getPort() << "\n";

	for(std::size_t i=0; i<depth; ++i) {
		logger.info << "|   ";
	}
	if(isHttps()) {
		logger.info << "HTTPS: YES\n";
	}
	else {
		logger.info << "HTTPS: NO\n";
	}

	for(auto& entry : listenerByHostname) {
		for(std::size_t i=0; i<depth; ++i) {
			logger.info << "|   ";
		}
		logger.info << "+-> Hostname: \"" << entry.first << "\"\n";

		entry.second->dumpTree(depth + 1);
	}
}

void Socket::addListener(Listener& listener) {
	if(listenerByHostname.count(listener.getHostname()) > 0) {
		throw esl::addStacktrace(std::runtime_error("Cannot add listener for hostname \"" + listener.getHostname() + "\" to http-server \"" + getId() + "\" because there is already another listener added for same hostname."));

	}
	listenerByHostname[listener.getHostname()] = &listener;
	refListeners.push_back(std::ref(listener));
}

esl::http::server::Interface::Socket& Socket::getSocket() noexcept {
	return socket;
}

const std::string& Socket::getId() const noexcept {
	return id;
}

const std::string& Socket::getImplementation() const noexcept {
	return implementation;
}

std::uint16_t Socket::getPort() const noexcept {
	return port;
}

bool Socket::isHttps() const noexcept {
	return https;
}

std::set<std::string> Socket::getHostnames() const {
	std::set<std::string> hostnames;

	for(auto& entry : listenerByHostname) {
		hostnames.insert(entry.first);
	}

	return hostnames;
}

Listener* Socket::getListenerByHostname(const std::string& hostname) {
	auto iter = listenerByHostname.find(hostname);
	if(iter == std::end(listenerByHostname)) {
		return nullptr;
	}
	return iter->second;
}

esl::io::Input Socket::createRequestHandler(esl::http::server::RequestContext& baseRequestContext) {
	/* Access log */
	logger.info << "Request for hostname " << baseRequestContext.getRequest().getHostName() << ": " << baseRequestContext.getRequest().getMethod() << " \"" << baseRequestContext.getRequest().getPath() << "\" received from " << baseRequestContext.getRequest().getRemoteAddress() << "\n";

	Socket* socket = baseRequestContext.findObject<Socket>("");
	if(!socket) {
		logger.info << "ERROR: No http-server available for request to " << baseRequestContext.getRequest().getHostName() << ":" << baseRequestContext.getRequest().getHostPort() << "\n";

		ExceptionHandler exceptionHandler;

		exceptionHandler.setShowException(false);
		exceptionHandler.setShowStacktrace(false);
		//esl::http::server::exception::StatusCode e(500, "Engine object not found");
		//exceptionHandler.setMessage(e);
		exceptionHandler.call([]() {
			throw esl::http::server::exception::StatusCode(500, "Engine object not found");
		});

		// TODO
		// This cast is a workaround to avoid a compile time error.
		// Why does the compiler not find the matching dump-method in the base class?
		//exceptionHandler.dump(logger.error);
    	engine::ExceptionHandler& e(exceptionHandler);
    	e.dump(logger.error);

		exceptionHandler.dump(baseRequestContext.getConnection());
		return esl::io::Input();
	}
	logger.debug << "Http-server found with id " << socket->getId() << "\"\n";


	/* *************************************************** *
	 * Lookup corresponding Listener matching the hostname *
	 * *************************************************** */
	logger.debug << "Lookup http-listerner by host name \"" << baseRequestContext.getRequest().getHostName() << "\"\n";

	std::string hostname = baseRequestContext.getRequest().getHostName();
	Listener* listener = socket->getListenerByHostname(hostname);
	while(!listener) {
		std::string::size_type pos = hostname.find_first_of('.');
		if(pos == std::string::npos) {
			logger.debug << "Lookup http-listerner by host name \"*\"\n";
			listener = socket->getListenerByHostname("*");
			break;
		}
		hostname = hostname.substr(pos+1);
		logger.debug << "Lookup http-listerner by host name \"*." << hostname << "\"\n";
		listener = socket->getListenerByHostname("*." + hostname);
	}

	if(listener == nullptr) {
		/* Error log */
		logger.info << "ERROR: No listener available for invalid request " << baseRequestContext.getRequest().getHostName() << ":" << baseRequestContext.getRequest().getHostPort() << "\n";

		//esl::http::server::exception::StatusCode e(404);
		ExceptionHandler exceptionHandler;

		exceptionHandler.setShowException(false);
		exceptionHandler.setShowStacktrace(false);
		//exceptionHandler.setMessage(esl::http::server::exception::StatusCode(500));
		exceptionHandler.call([]() {
			throw esl::http::server::exception::StatusCode(500);
		});
		exceptionHandler.dump(baseRequestContext.getConnection());

		return esl::io::Input();
	}

	logger.debug << "Http-listener found\n";
	return listener->createRequestHandler(baseRequestContext);
}

} /* namespace server */
} /* namespace http */
} /* namespace engine */
} /* namespace jerry */
