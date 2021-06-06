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

#include <jerry/builtin/basic/echo/Settings.h>
#include <jerry/Logger.h>

#include <esl/Stacktrace.h>

#include <stdexcept>

namespace jerry {
namespace builtin {
namespace basic {
namespace echo {

namespace {
Logger logger("jerry::builtin::basic::echo::Settings");
}

std::unique_ptr<esl::object::Interface::Object> Settings::create() {
	return std::unique_ptr<esl::object::Interface::Object>(new Settings);
}

void Settings::addSetting(const std::string& key, const std::string& value) {
	if(key == "notifier") {
		notifiers.insert(value);
	}
	else if(key == "delay-ms") {
		msDelay = std::stoul(value);
	}
	else if(key == "output.ref-id") {
		outputRefId = value;
	}
	else if(key.substr(0, 17) == "output.parameter.") {
		outputParameters.push_back(std::make_pair(key.substr(17), value));
	}
	else {
		throw esl::addStacktrace(std::runtime_error("Unknown parameter key=\"" + key + "\" with value=\"" + value + "\""));
	}
}

void Settings::initializeContext(esl::object::ObjectContext& objectContext) {
	client = objectContext.findObject<esl::com::basic::broker::Interface::Client>(outputRefId);
	if(client == nullptr) {
		throw esl::addStacktrace(std::runtime_error("Cannot find basic-broker with id \"" + outputRefId + "\""));
	}

	logger.info << "Echo::Setting initialized for refId \"" << outputRefId << "\"\n";
}


const std::set<std::string>& Settings::getNotifiers() const noexcept {
	return notifiers;
}

std::unique_ptr<esl::com::basic::client::Interface::Connection> Settings::createConnection() {
	if(client == nullptr) {
		logger.error << "client == nullptr\n";
		return nullptr;
	}
	return client->createConnection(outputParameters);
}

unsigned long Settings::getMSDelay() const {
	return msDelay;
}

} /* namespace echo */
} /* namespace basic */
} /* namespace builtin */
} /* namespace jerry */
