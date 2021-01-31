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

#include <jerry/engine/messaging/proxy/Client.h>
#include <jerry/Logger.h>

#include <esl/Stacktrace.h>

#include <stdexcept>

namespace jerry {
namespace engine {
namespace messaging {
namespace proxy {

namespace {
Logger logger("jerry::engine::messaging::proxy::Client");
} /* anonymous namespace */

Consumer Client::consumer;

Client::Client(const std::string& aId, const std::string& aBrokers, std::uint16_t aThreads, const std::vector<std::pair<std::string, std::string>>& aSettings, const std::string& aImplementation)
: id(aId),
  brokers(aBrokers),
  threads(aThreads),
  implementation(aImplementation)
{
	for(const auto& setting : aSettings) {
		settings.addSetting(setting.first, setting.second);
	}
}

void Client::addObjectFactory(const std::string& id, ObjectFactory objectFactory) {
	throw esl::addStacktrace(std::runtime_error("Adding objectFactory is not allowed."));
}

esl::messaging::Consumer& Client::getConsumer() {
	return consumer;
}

std::unique_ptr<esl::messaging::Interface::ProducerFactory> Client::createProducerFactory(const std::string& id, std::vector<std::pair<std::string, std::string>> parameters) {
	if(getBaseClient() == nullptr) {
		throw esl::addStacktrace(std::runtime_error("Cannot create producer factory with empty base client."));
	}
	return getBaseClient()->createProducerFactory(id, std::move(parameters));
}

esl::messaging::Client* Client::getBaseClient() const {
	return client.get();
}

} /* namespace proxy */
} /* namespace messaging */
} /* namespace engine */
} /* namespace jerry */
