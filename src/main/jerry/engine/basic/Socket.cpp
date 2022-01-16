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

#include <jerry/engine/basic/Socket.h>
//#include <jerry/ExceptionHandler.h>
#include <jerry/Logger.h>

#include <esl/Stacktrace.h>

#include <stdexcept>

namespace jerry {
namespace engine {
namespace basic {


namespace {
Logger logger("jerry::engine::basic::Socket");
}

Socket::Socket(const esl::object::Interface::Settings& aSettings, const std::string& aImplementation)
: socket(aSettings, aImplementation),
  requestHandler(*this),
  implementation(aImplementation),
  settings(aSettings)
{ }


void Socket::listen(std::function<void()> onReleasedHandler) {
	socket.listen(requestHandler, onReleasedHandler);
}

std::set<std::string> Socket::getNotifiers() const {
	return context.getNotifiers();
}

Context& Socket::getContext() noexcept {
	return context;
}

void Socket::listen(const esl::com::basic::server::requesthandler::Interface::RequestHandler& requestHandler, std::function<void()> onReleasedHandler) {
	logger.warn << "listen 'listen' is not allowed.\n";
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

void Socket::initializeContext() {
	getContext().initializeContext();
}

void Socket::dumpTree(std::size_t depth) const {
	for(std::size_t i=0; i<depth; ++i) {
		logger.info << "|   ";
	}
	logger.info << "Implementation: \"" << implementation << "\"\n";

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

	for(std::size_t i=0; i<depth; ++i) {
		logger.info << "|   ";
	}
	logger.info << "+-> Listener:\n";
	//logger.info << "Listener:\n";
	context.dumpTree(depth+1);
}


} /* namespace basic */
} /* namespace engine */
} /* namespace jerry */
