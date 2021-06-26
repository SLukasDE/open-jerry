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

Settings::Settings(const esl::object::Interface::Settings& settings) {
	for(const auto& setting : settings) {
		if(setting.first == "notifier") {
			notifiers.insert(setting.second);
		}
		else if(setting.first == "delay-ms") {
			try {
				msDelay = std::stoul(setting.second);
			}
			catch(...) {
				throw esl::addStacktrace(std::runtime_error("Invalid value \"" + setting.second + "\" for parameter key=\"" + setting.first + "\". Value must be an integer"));
			}
		}
		else if(setting.first == "output.ref-id") {
			outputRefId = setting.second;
		}
		else if(setting.first.substr(0, 17) == "output.parameter.") {
			outputParameters.push_back(std::make_pair(setting.first.substr(17), setting.second));
		}
		else {
			throw esl::addStacktrace(std::runtime_error("Unknown parameter key=\"" + setting.first + "\" with value=\"" + setting.second + "\""));
		}
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
