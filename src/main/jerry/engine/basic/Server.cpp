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

#include <jerry/engine/basic/Server.h>
#include <jerry/Logger.h>

#include <esl/Stacktrace.h>

#include <stdexcept>

namespace jerry {
namespace engine {
namespace basic {


namespace {
Logger logger("jerry::engine::basic::Server");
}

Server::Server(ProcessRegistry& aProcessRegistry, const std::vector<std::pair<std::string, std::string>>& aSettings, const std::string& aImplementation)
: socket(aSettings, aImplementation),
  requestHandler(*this),
  implementation(aImplementation),
  settings(aSettings),
  processRegistry(aProcessRegistry),
  context(processRegistry)
{ }

void Server::initializeContext() {
	getContext().initializeContext();
}

void Server::procedureRun(esl::object::ObjectContext&) {
	try {
		processRegistry.processRegister(*this);
		socket.listen(requestHandler, [this] {
			processRegistry.processUnregister(*this);
		});
	}
	catch(...) {
		processRegistry.processUnregister(*this);
		throw;
	}
}

void Server::procedureCancel()  {
	socket.release();
}

Context& Server::getContext() noexcept {
	return context;
}

void Server::dumpTree(std::size_t depth) const {
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

	context.dumpTree(depth+1);
}

} /* namespace basic */
} /* namespace engine */
} /* namespace jerry */
