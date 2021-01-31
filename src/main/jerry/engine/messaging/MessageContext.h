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

#ifndef JERRY_ENGINE_MESSAGING_MESSAGECONTEXT_H_
#define JERRY_ENGINE_MESSAGING_MESSAGECONTEXT_H_

#include <jerry/engine/messaging/MessageContext.h>
#include <jerry/engine/messaging/Message.h>

#include <esl/messaging/MessageContext.h>
#include <esl/messaging/Message.h>
#include <esl/object/Interface.h>
#include <esl/object/ObjectContext.h>

#include <string>
#include <functional>

namespace jerry {
namespace engine {
namespace messaging {

class MessageContext : public esl::messaging::MessageContext {
public:
	MessageContext(esl::messaging::MessageContext& messageContext);

	void setObjectContext(const esl::object::ObjectContext& objectContext);
	Message& getEngineMessage() const;

	esl::messaging::Message& getMessage() const override;

	esl::object::Interface::Object* findObject(const std::string& id) const override;

private:
	esl::messaging::MessageContext& messageContext;
	std::reference_wrapper<const esl::object::ObjectContext> objectContext;
	Message message;
};

} /* namespace messaging */
} /* namespace engine */
} /* namespace jerry */

#endif /* JERRY_ENGINE_MESSAGING_MESSAGECONTEXT_H_ */
