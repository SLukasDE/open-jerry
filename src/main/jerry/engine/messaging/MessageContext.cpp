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

#include <jerry/engine/messaging/MessageContext.h>
#include <jerry/Logger.h>

namespace jerry {
namespace engine {
namespace messaging {

namespace {
Logger logger("jerry::engine::messaging::MessageContext");


class EmptyContext : public esl::object::ObjectContext {
public:
	esl::object::Interface::Object* findObject(const std::string& id = "") const {
		return nullptr;
	}
};

esl::object::ObjectContext& getEmptyContext() {
	static EmptyContext emptyContext;
	return emptyContext;
}
}

MessageContext::MessageContext(esl::messaging::MessageContext& aMessageContext)
: messageContext(aMessageContext),
  objectContext(std::cref(getEmptyContext())),
  message(messageContext.getMessage())
{ }

void MessageContext::setObjectContext(const esl::object::ObjectContext& aObjectContext) {
	message.getReaderBuffered().reset();
	objectContext = std::ref(aObjectContext);
}

Message& MessageContext::getEngineMessage() const {
	return const_cast<Message&>(message);
}

esl::messaging::Message& MessageContext::getMessage() const {
	return getEngineMessage();
}

esl::object::Interface::Object* MessageContext::findObject(const std::string& id) const {
	esl::object::Interface::Object* object = objectContext.get().findObject(id);
	if(object == nullptr) {
		object = messageContext.findObject(id);
	}
	return object;
}

} /* namespace messaging */
} /* namespace engine */
} /* namespace jerry */
