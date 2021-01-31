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

#include <jerry/engine/messaging/Listener.h>
#include <jerry/Logger.h>

#include <stdexcept>

namespace jerry {
namespace engine {
namespace messaging {

namespace {
Logger logger("jerry::engine::messaging::Listener");
}

Listener::Listener(esl::object::ObjectContext& aEngineContext,
		std::map<std::string, std::vector<MessageBrokerHandler>>& aHandlerByQueueName,
		bool inheritObjects)
: Context(*this, nullptr, nullptr, inheritObjects),
  engineContext(aEngineContext),
  handlerByQueueName(aHandlerByQueueName)
{ }

esl::object::Interface::Object* Listener::findHiddenObject(const std::string& id) const {
	esl::object::Interface::Object* object = Context::findLocalObject(id);

	if(object) {
		return object;
	}

	return engineContext.findObject(id);
}

void Listener::registerHandlerData(const std::string& queueName, esl::messaging::messagehandler::Interface::CreateMessageHandler createMessageHandler, const Context& context) {
	handlerByQueueName[queueName].push_back(MessageBrokerHandler(createMessageHandler, context));
}

} /* namespace messaging */
} /* namespace engine */
} /* namespace jerry */
