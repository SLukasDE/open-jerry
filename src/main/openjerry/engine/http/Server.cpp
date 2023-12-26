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

#include <openjerry/engine/http/Server.h>
#include <openjerry/engine/http/RequestContext.h>
#include <openjerry/engine/ProcessRegistry.h>
#include <openjerry/Logger.h>

#include <esl/com/http/server/MHDSocket.h>
#include <esl/system/Stacktrace.h>
#include <esl/plugin/Registry.h>

#include <stdexcept>

namespace openjerry {
namespace engine {
namespace http {


namespace {
Logger logger("openjerry::engine::http::Server");
}

Server::Server(ProcessRegistry& aProcessRegistry, const std::vector<std::pair<std::string, std::string>>& aSettings, const std::string& aImplementation)
#if 0
: socket(esl::plugin::Registry::get().create<esl::com::http::server::Socket>(aImplementation.empty() ? "esl/com/http/server/MHDSocket" : aImplementation, aSettings)),
#else
: socket(aImplementation.empty() ? esl::com::http::server::MHDSocket::create(aSettings) : esl::plugin::Registry::get().create<esl::com::http::server::Socket>(aImplementation, aSettings)),
#endif
  processRegistry(aProcessRegistry),
  context(nullptr),
  requestHandler(context),
  implementation(aImplementation),
  settings(aSettings)
{
	if(socket.get() == nullptr) {
		throw esl::system::Stacktrace::add(std::runtime_error("Could not create an http socket with implementation \"" + implementation + "\""));
	}
	context.setProcessRegistry(&processRegistry);
}

void Server::initializeContext() {
	getContext().initializeContext();
}

void Server::procedureRun(esl::object::Context&) {
	try {
		processRegistry.processRegister(*this);
		socket->listen(requestHandler, [this] {
			processRegistry.processUnregister(*this);
		});
	}
	catch(...) {
		processRegistry.processUnregister(*this);
		throw;
	}
}

void Server::procedureCancel()  {
	socket->release();
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

} /* namespace http */
} /* namespace engine */
} /* namespace openjerry */
