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

#include <jerry/engine/http/Server.h>
#include <jerry/engine/http/RequestContext.h>
#include <jerry/engine/ProcessRegistry.h>
#include <jerry/Logger.h>

//#include <esl/stacktrace/Stacktrace.h>
#include <esl/plugin/Registry.h>

//#include <memory>
#include <stdexcept>

namespace jerry {
namespace engine {
namespace http {


namespace {
Logger logger("jerry::engine::http::Server");
}

Server::Server(ProcessRegistry& aProcessRegistry, bool aHttps, const std::vector<std::pair<std::string, std::string>>& aSettings, const std::string& aImplementation)
: socket(esl::plugin::Registry::get().create<esl::com::http::server::Socket>(aImplementation, aSettings)),
  processRegistry(aProcessRegistry),
  context(nullptr),
  requestHandler(context),
  https(aHttps),
  implementation(aImplementation),
  settings(aSettings)
{
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

void Server::addTLSHost(const std::string& hostname, std::vector<unsigned char> certificate, std::vector<unsigned char> key) {
	socket->addTLSHost(hostname, std::move(certificate), std::move(key));
}

bool Server::isHttps() const noexcept {
	return https;
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

	context.dumpTree(depth+1);
}

} /* namespace http */
} /* namespace engine */
} /* namespace jerry */
