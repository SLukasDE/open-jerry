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
#if 0
#include <jerry/builtin/messaging/ProducerFactory.h>
#include <jerry/Logger.h>

#include <esl/Stacktrace.h>

#include <stdexcept>

namespace jerry {
namespace builtin {
namespace messaging {

namespace {
Logger logger("jerry::builtin::messaging::ProducerFactory");
}

std::unique_ptr<esl::object::Interface::Object> ProducerFactory::create() {
	return std::unique_ptr<esl::object::Interface::Object>(new ProducerFactory);
}

void ProducerFactory::addSetting(const std::string& key, const std::string& value) {
	if(key == "broker") {
		messageBrokerId = value;
	}
	else if(key == "queue") {
		queue = value;
	}
	else {
		parameters.push_back(std::make_pair(key, value));
	}
}

void ProducerFactory::initializeContext(esl::object::ObjectContext& objectContext) {
	messageBroker = objectContext.findObject<esl::messaging::Interface::Client>(messageBrokerId);
	if(messageBroker == nullptr) {
		throw esl::addStacktrace(std::runtime_error("Cannot find message broker with id \"" + messageBrokerId + "\""));
	}

	logger.info << "ProducerFactory initialized for message broker \"" << messageBrokerId << "\"\n";
}

std::unique_ptr<esl::messaging::Producer> ProducerFactory::createProducer() {
	if(producerFactoryCreated == false) {
		producerFactoryCreated = true;
		producerFactory = messageBroker->createProducerFactory(queue, std::move(parameters));
		logger.info << "ProducerFactory: createProducerFactory(\"" << queue << "\")\n";
		if(producerFactory == nullptr) {
			logger.info << "ProducerFactory: createProducerFactory(\"" << queue << "\") failed\n";
		}
	}

	if(producerFactory == nullptr) {
		return nullptr;
	}
	return producerFactory->createProducer();
}

} /* namespace messaging */
} /* namespace builtin */
} /* namespace jerry */
#endif
