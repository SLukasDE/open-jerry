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

#include <jerry/engine/messaging/MessageHandler.h>
#include <jerry/engine/messaging/MessageContext.h>
#include <jerry/engine/messaging/Message.h>
#include <jerry/engine/Engine.h>
#include <jerry/engine/ExceptionHandler.h>
#include <jerry/Logger.h>

#include <esl/Stacktrace.h>

#include <stdexcept>

namespace jerry {
namespace engine {
namespace messaging {

namespace {
Logger logger("jerry::engine::messaging::MessageHandler");
}

std::unique_ptr<esl::utility::Consumer> MessageHandler::create(
		esl::messaging::MessageContext& baseMessageContext,
		std::vector<proxy::Client::Handler>& messageBrokerHandlers) {
	std::unique_ptr<MessageContext> messageContext(new MessageContext(baseMessageContext));
	std::size_t handlerDataIndex = 0;
	std::unique_ptr<esl::utility::Consumer> currentHandler;
	bool exceptionOccurred = false;

	for(handlerDataIndex = 0; handlerDataIndex < messageBrokerHandlers.size(); ++handlerDataIndex) {
		ExceptionHandler exceptionHandler;

		messageContext->setObjectContext(messageBrokerHandlers[handlerDataIndex].getContext());

		exceptionHandler.setShowException(true);
		exceptionHandler.setShowStacktrace(true);
		if(exceptionHandler.call([&messageContext, &currentHandler, &messageBrokerHandlers, handlerDataIndex]() {
			currentHandler = messageBrokerHandlers[handlerDataIndex].getCreateMessageHandler()(*messageContext);
		})) {
			logger.error << "Exception occurred on creating message handler for message from queue \"" << baseMessageContext.getMessage().getId() << "\".\n";
			exceptionHandler.dump(logger.error);
			currentHandler.reset();
			exceptionOccurred = true;
			break;
		}

		if(currentHandler) {
			break;
		}
	}

	if(exceptionOccurred) {
		return nullptr;
	}

	if(currentHandler == nullptr) {
		logger.warn << "No message handler created for message from queue \"" << baseMessageContext.getMessage().getId() << "\".\n";
		return nullptr;
	}

	logger.info << "Message handler created for message from queue \"" << baseMessageContext.getMessage().getId() << "\".\n";
	return std::unique_ptr<esl::utility::Consumer>(new MessageHandler(std::move(messageContext), messageBrokerHandlers, handlerDataIndex, std::move(currentHandler)));
}

MessageHandler::MessageHandler(
		std::unique_ptr<MessageContext> aMessageContext,
		std::vector<proxy::Client::Handler>& aMessageBrokerHandlers,
		std::size_t aHandlerDataIndex,
		std::unique_ptr<esl::utility::Consumer> aCurrentHandler)
: messageContext(std::move(aMessageContext)),
  messageBrokerHandlers(aMessageBrokerHandlers),
  handlerDataIndex(aHandlerDataIndex),
  currentHandler(std::move(aCurrentHandler))
{ }

std::size_t MessageHandler::read(esl::utility::Reader& reader) {
	messageContext->getEngineMessage().getReaderBuffered().setBaseReader(reader);

	for(; handlerDataIndex < messageBrokerHandlers.size(); ++handlerDataIndex) {
		ExceptionHandler exceptionHandler;
		exceptionHandler.setShowException(true);
		exceptionHandler.setShowStacktrace(true);

		if(currentHandler == nullptr) {
			messageContext->setObjectContext(messageBrokerHandlers[handlerDataIndex].getContext());

			if(exceptionHandler.call([this]() {
				currentHandler = messageBrokerHandlers[handlerDataIndex].getCreateMessageHandler()(*messageContext);
			})) {
				currentHandler.reset();
				logger.error << "Exception occurred on creating message handler for message from queue \"" << messageContext->getMessage().getId() << "\".\n";
				exceptionHandler.dump(logger.error);
		        throw esl::addStacktrace(std::runtime_error("Exception occurred on creating message handler for message from queue \"" + messageContext->getMessage().getId() + "\""));
			}
		}

		std::size_t readResult = 0;
		while(readResult != esl::utility::Reader::npos) {
			if(exceptionHandler.call([this, &readResult]() {
				readResult = currentHandler->read(messageContext->getEngineMessage().getReaderBuffered());
			})) {
				readResult = esl::utility::Reader::npos;
				logger.error << "Exception occurred on reading message from queue \"" << messageContext->getMessage().getId() << "\".\n";
				exceptionHandler.dump(logger.error);
		        throw esl::addStacktrace(std::runtime_error("Exception occurred on reading message from queue \"" + messageContext->getMessage().getId() + "\""));
			}

			if(readResult == 0) {
				return 0;
			}
		}
		currentHandler.reset();
	}

	return esl::utility::Reader::npos;
}

} /* namespace messaging */
} /* namespace engine */
} /* namespace jerry */
