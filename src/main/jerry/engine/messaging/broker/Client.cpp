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

#include <jerry/engine/messaging/broker/Client.h>
#include <jerry/engine/messaging/server/RequestContext.h>
#include <jerry/Logger.h>

#include <esl/object/Properties.h>
#include <esl/Stacktrace.h>

#include <stdexcept>

namespace jerry {
namespace engine {
namespace messaging {
namespace broker {

namespace {
Logger logger("jerry::engine::messaging::broker::Client");
} /* anonymous namespace */

Client::Client(esl::object::ObjectContext& aEngineContext, const std::string& aId, const std::string& aBrokers, const std::vector<std::pair<std::string, std::string>>& settings, const std::string& aImplementation)
: client(aBrokers, esl::object::Properties(settings), aImplementation),
  socket(aEngineContext, aId, client.getSocket()),
  engineContext(aEngineContext),
  id(aId),
  brokers(aBrokers),
  implementation(aImplementation)
{
	getClient().getSocket().addObjectFactory("", [this](const esl::messaging::server::RequestContext&){ return this; });
}

esl::messaging::server::Interface::Socket& Client::getSocket() {
	return socket;
}

std::unique_ptr<esl::messaging::client::Interface::Connection> Client::createConnection(std::vector<std::pair<std::string, std::string>> parameters) {
	throw esl::addStacktrace(std::runtime_error("Calling 'createConnection' is not allowed."));
	return nullptr;
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
	logger.info << "Brokers: \"" << getBrokers() << "\"\n";
}

server::Socket& Client::getServer() noexcept {
	return socket;
}

esl::messaging::broker::Interface::Client& Client::getClient() noexcept {
	return client;
}

const std::string& Client::getId() const noexcept {
	return id;
}

const std::string& Client::getBrokers() const noexcept {
	return brokers;
}

const std::string& Client::getImplementation() const noexcept {
	return implementation;
}

} /* namespace broker */
} /* namespace messaging */
} /* namespace engine */
} /* namespace jerry */
