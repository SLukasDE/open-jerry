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

#ifndef JERRY_ENGINE_MESSAGING_LISTENER_H_
#define JERRY_ENGINE_MESSAGING_LISTENER_H_

#include <jerry/engine/messaging/Context.h>
#include <jerry/engine/messaging/Endpoint.h>
#include <jerry/engine/messaging/MessageBrokerHandler.h>

#include <esl/messaging/messagehandler/Interface.h>
#include <esl/object/ObjectContext.h>

#include <string>
#include <map>
#include <utility>
#include <vector>

namespace jerry {
namespace engine {
namespace messaging {

class Listener : public Context {
public:
	Listener(esl::object::ObjectContext& engineContext,
			std::map<std::string, std::vector<MessageBrokerHandler>>& handlerByQueueName,
			bool inheritObjects);

	esl::object::Interface::Object* findHiddenObject(const std::string& id) const override;

	void registerHandlerData(const std::string& queueName, esl::messaging::messagehandler::Interface::CreateMessageHandler createMessageHandler, const Context& context);

private:
	esl::object::ObjectContext& engineContext;
	std::map<std::string, std::vector<MessageBrokerHandler>>& handlerByQueueName;
};

} /* namespace messaging */
} /* namespace engine */
} /* namespace jerry */

#endif /* JERRY_ENGINE_MESSAGING_LISTENER_H_ */
