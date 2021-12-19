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
#include <jerry/engine/http/server/Context.h>
#include <jerry/engine/http/server/RequestContext.h>
#include <jerry/Logger.h>

#include <esl/Stacktrace.h>

#include <memory>
#include <stdexcept>

namespace jerry {
namespace engine {
namespace http {
namespace server {

namespace {
Logger logger("jerry::engine::http::server::Socket");
}

Socket::Socket(const std::string& aId, bool aHttps,
		const esl::object::Interface::Settings& aSettings,
		const std::string& aImplementation)
: socket(aSettings, aImplementation),
  requestHandler(*this),
  https(aHttps),
  id(aId),
  implementation(aImplementation),
  settings(aSettings)
{ }

void Socket::addTLSHost(const std::string& hostname, std::vector<unsigned char> certificate, std::vector<unsigned char> key) {
	socket.addTLSHost(hostname, std::move(certificate), std::move(key));
	//logger.warn << "Calling 'addTLSHost' is not allowed.\n";
	//throw esl::addStacktrace(std::runtime_error("Calling 'addTLSHost' is not allowed."));
}

void Socket::listen(std::function<void()> onReleasedHandler) {
	socket.listen(requestHandler, onReleasedHandler);
}

void Socket::addListener(Listener& listener) {
	if(listenerByHostname.count(listener.getHostname()) > 0) {
		throw esl::addStacktrace(std::runtime_error("Cannot add http-listener for hostname \"" + listener.getHostname() + "\" because there is already another listener added for same hostname."));

	}
	listenerByHostname[listener.getHostname()] = &listener;
	refListeners.push_back(std::ref(listener));
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

Listener* Socket::getListenerByHostname(std::string hostname) const {
	auto iter = listenerByHostname.find(hostname);
	while(iter == std::end(listenerByHostname)) {
		std::string::size_type pos = hostname.find_first_of('.');
		if(pos == std::string::npos) {
			logger.debug << "Lookup http-listerner by host name \"*\"\n";
			iter = listenerByHostname.find("*");
			break;
		}
		hostname = hostname.substr(pos+1);
		logger.debug << "Lookup http-listerner by host name \"*." << hostname << "\"\n";
		iter = listenerByHostname.find("*." + hostname);
	}

	if(iter == std::end(listenerByHostname)) {
		return nullptr;
	}
	return iter->second;
}

void Socket::listen(const esl::com::http::server::requesthandler::Interface::RequestHandler& requestHandler, std::function<void()> onReleasedHandler) {
	logger.warn << "Calling 'listen' is not allowed.\n";
	//throw esl::addStacktrace(std::runtime_error("Calling 'listen' is not allowed."));
}

void Socket::release() {
	socket.release();
	//logger.warn << "Calling 'release' is not allowed.\n";
	//throw esl::addStacktrace(std::runtime_error("Calling 'release' is not allowed."));
}

bool Socket::wait(std::uint32_t ms) {
	return socket.wait(ms);
	//logger.warn << "Calling 'wait' is not allowed.\n";
	//throw esl::addStacktrace(std::runtime_error("Calling 'release' is not allowed."));
	//return false;
}

void Socket::dumpTree(std::size_t depth) const {
	for(std::size_t i=0; i<depth; ++i) {
		logger.info << "|   ";
	}
	logger.info << "+-> ID: \"" << getId() << "\" -> " << this << "\n";
	++depth;

	for(std::size_t i=0; i<depth; ++i) {
		logger.info << "|   ";
	}
	logger.info << "Implementation: \"" << getImplementation() << "\"\n";

    for(std::size_t i=0; i<depth; ++i) {
            logger.info << "|   ";
    }
    if(isHttps()) {
    	logger.info << "HTTPS: YES\n";
    }
    else {
    	logger.info << "HTTPS: NO\n";
    }

	for(std::size_t i=0; i<depth; ++i) {
		logger.info << "|   ";
	}
	logger.info << "+-> Parameters:\n";

	for(const auto& setting : settings) {
		for(std::size_t i=0; i<depth+1; ++i) {
			logger.info << "|   ";
		}
		logger.info << "key: \"" << setting.first << "\" = value: \"" << setting.second << "\"\n";
	}

	std::set<std::string> hostnames = getHostnames();
	for(const auto& hostname : hostnames) {
		http::server::Listener* listener = getListenerByHostname(hostname);
		if(listener == nullptr) {
			continue;
		}

		for(std::size_t i=0; i<depth; ++i) {
			logger.info << "|   ";
		}
		logger.info << "+-> Hostname: \"" << hostname << "\" -> " << listener << "\n";
	}
}

const std::string& Socket::getId() const noexcept {
	return id;
}

const std::string& Socket::getImplementation() const noexcept {
	return implementation;
}

} /* namespace server */
} /* namespace http */
} /* namespace engine */
} /* namespace jerry */
