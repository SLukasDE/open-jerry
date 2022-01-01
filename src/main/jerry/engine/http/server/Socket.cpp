/*
 * This file is part of Jerry application server.
 * Copyright (C) 2020-2022 Sven Lukas
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

Socket::Socket(bool aHttps, const esl::object::Interface::Settings& aSettings, const std::string& aImplementation)
: socket(aSettings, aImplementation),
  requestHandler(*this),
  https(aHttps),
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

bool Socket::isHttps() const noexcept {
	return https;
}

Context& Socket::getContext() noexcept {
	return context;
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

//void Socket::initializeContext(esl::object::Interface::ObjectContext& objectContext) {
void Socket::initializeContext() {
	getContext().initializeContext();
}

void Socket::dumpTree(std::size_t depth) const {
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

	for(std::size_t i=0; i<depth+1; ++i) {
		logger.info << "|   ";
	}
	logger.info << "Listener:\n";
	context.dumpTree(depth+1);
}

const std::string& Socket::getImplementation() const noexcept {
	return implementation;
}

} /* namespace server */
} /* namespace http */
} /* namespace engine */
} /* namespace jerry */
