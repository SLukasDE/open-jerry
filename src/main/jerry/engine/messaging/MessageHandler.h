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

#ifndef JERRY_ENGINE_MESSAGING_MESSAGEHANDLER_H_
#define JERRY_ENGINE_MESSAGING_MESSAGEHANDLER_H_

#include <jerry/engine/messaging/MessageContext.h>
#include <jerry/engine/messaging/proxy/Client.h>

#include <esl/utility/Consumer.h>
#include <esl/utility/Reader.h>
#include <esl/utility/ReaderBuffered.h>
#include <esl/messaging/messagehandler/Interface.h>
#include <esl/messaging/MessageContext.h>
#include <esl/Stacktrace.h>

#include <string>
#include <vector>
#include <utility>
#include <memory>

namespace jerry {
namespace engine {
namespace messaging {

class Context;
class Endpoint;
class Listener;

class MessageHandler : public esl::utility::Consumer {
public:
	static std::unique_ptr<esl::utility::Consumer> create(
			esl::messaging::MessageContext& baseMessageContext,
			std::vector<proxy::Client::Handler>& messageBrokerHandlers);

	MessageHandler(
			std::unique_ptr<MessageContext> messageContext,
			std::vector<proxy::Client::Handler>& messageBrokerHandlers,
			std::size_t handlerDataIndex,
			std::unique_ptr<esl::utility::Consumer> currentHandler);

	std::size_t read(esl::utility::Reader& reader) override;

private:
	std::unique_ptr<MessageContext> messageContext;

	std::vector<proxy::Client::Handler>& messageBrokerHandlers;
	std::size_t handlerDataIndex = 0;
	std::unique_ptr<esl::utility::Consumer> currentHandler;
};

} /* namespace messaging */
} /* namespace engine */
} /* namespace jerry */

#endif /* JERRY_ENGINE_MESSAGING_MESSAGEHANDLER_H_ */
