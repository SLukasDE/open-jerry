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

#include <jerry/engine/basic/broker/Client.h>
#include <jerry/engine/basic/server/RequestContext.h>
#include <jerry/Logger.h>

#include <esl/Stacktrace.h>

#include <stdexcept>

namespace jerry {
namespace engine {
namespace basic {
namespace broker {

namespace {
Logger logger("jerry::engine::messaging::broker::Client");
} /* anonymous namespace */

Client::Client(esl::object::ObjectContext& aEngineContext, const std::string& aId, const esl::object::Interface::Settings& aSettings, const std::string& aImplementation)
: client(aSettings, aImplementation),
  socket(aEngineContext, aId, client.getSocket()),
  engineContext(aEngineContext),
  id(aId),
  implementation(aImplementation),
  settings(aSettings)
{
	getClient().getSocket().addObjectFactory("", [this](const esl::com::basic::server::RequestContext&){ return this; });
}

esl::com::basic::server::Interface::Socket& Client::getSocket() {
	return socket;
}

std::unique_ptr<esl::com::basic::client::Interface::Connection> Client::createConnection(const esl::object::Interface::Settings& parameters) {
	//throw esl::addStacktrace(std::runtime_error("Calling 'createConnection' is not allowed."));
	//return nullptr;
	return client.createConnection(parameters);
}

void Client::dumpTree(std::size_t depth) const {
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
	logger.info << "+-> Parameters:\n";
	++depth;

	for(const auto& setting : settings) {
		for(std::size_t i=0; i<depth; ++i) {
			logger.info << "|   ";
		}
		logger.info << "key: \"" << setting.first << "\" = value: \"" << setting.second << "\"\n";
	}
}

server::Socket& Client::getServer() noexcept {
	return socket;
}

esl::com::basic::broker::Interface::Client& Client::getClient() noexcept {
	return client;
}

const std::string& Client::getId() const noexcept {
	return id;
}

const std::string& Client::getImplementation() const noexcept {
	return implementation;
}

} /* namespace broker */
} /* namespace basic */
} /* namespace engine */
} /* namespace jerry */
