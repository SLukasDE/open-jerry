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

#include <jerry/builtin/messaging/echo/Settings.h>
#include <jerry/Logger.h>

#include <esl/Stacktrace.h>

#include <stdexcept>
//#include <cstdlib>


namespace jerry {
namespace builtin {
namespace messaging {
namespace echo {

namespace {
Logger logger("jerry::builtin::messaging::echo::Settings");
}

std::unique_ptr<esl::object::Interface::Object> Settings::create() {
	return std::unique_ptr<esl::object::Interface::Object>(new Settings);
}

void Settings::addSetting(const std::string& key, const std::string& value) {
	if(key == "queue") {
		queue = value;
	}
	else if(key == "delay-ms") {
		msDelay = std::stoul(value);
	}
	else {
		throw esl::addStacktrace(std::runtime_error("Unknown parameter key=\"" + key + "\" with value=\"" + value + "\""));
	}
}

void Settings::initializeContext(esl::object::ObjectContext& objectContext) {
	producerFactory = objectContext.findObject<esl::messaging::Interface::ProducerFactory>(queue);
	if(producerFactory == nullptr) {
		throw esl::addStacktrace(std::runtime_error("Cannot find message producer factory for echo queue \"" + queue + "\""));
	}

	logger.info << "Echo::Setting initialized for queue \"" << queue << "\"\n";
}

std::unique_ptr<esl::messaging::Producer> Settings::createProducer() {
	if(producerFactory == nullptr) {
		logger.error << "producerFactory == nullptr\n";
		return nullptr;
	}
	return producerFactory->createProducer();
}

unsigned long Settings::getMSDelay() const {
	return msDelay;
}

} /* namespace echo */
} /* namespace messaging */
} /* namespace builtin */
} /* namespace jerry */
