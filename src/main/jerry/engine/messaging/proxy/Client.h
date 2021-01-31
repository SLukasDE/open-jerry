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

#ifndef JERRY_ENGINE_MESSAGING_PROXY_CLIENT_H_
#define JERRY_ENGINE_MESSAGING_PROXY_CLIENT_H_

#include <jerry/engine/messaging/proxy/Consumer.h>
#include <jerry/engine/messaging/MessageBrokerHandler.h>
#include <jerry/engine/messaging/Listener.h>

#include <esl/messaging/Interface.h>
#include <esl/messaging/Client.h>
#include <esl/messaging/Consumer.h>
#include <esl/messaging/Producer.h>
#include <esl/object/Interface.h>
#include <esl/object/ValueSettings.h>

#include <cstdint>
#include <string>
#include <vector>
#include <map>
#include <utility>
#include <memory>

namespace jerry {
namespace engine {
namespace messaging {
namespace proxy {

class Client : public esl::messaging::Interface::Client {
public:
	using Handler = MessageBrokerHandler;

	Client(const std::string& id, const std::string& brokers, std::uint16_t threads, const std::vector<std::pair<std::string, std::string>>& settings, const std::string& implementation);

	void addObjectFactory(const std::string& id, ObjectFactory objectFactory) override;
	esl::messaging::Consumer& getConsumer() override;
	std::unique_ptr<esl::messaging::Interface::ProducerFactory> createProducerFactory(const std::string& id, std::vector<std::pair<std::string, std::string>> parameters) override;

	esl::messaging::Client* getBaseClient() const;

	std::string id;
	std::string brokers;
	std::uint16_t threads;
	std::string implementation;
	esl::object::ValueSettings settings;

	std::unique_ptr<esl::messaging::Client> client;
	std::unique_ptr<esl::object::Interface::Object> object;

	std::vector<std::unique_ptr<Listener>> listeners;

	std::map<std::string, std::vector<Handler>> handlerByQueueName;

private:
	static Consumer consumer;
};

} /* namespace proxy */
} /* namespace messaging */
} /* namespace engine */
} /* namespace jerry */

#endif /* JERRY_ENGINE_MESSAGING_PROXY_CLIENT_H_ */
