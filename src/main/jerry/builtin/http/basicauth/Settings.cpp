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

#include <jerry/builtin/http/basicauth/Settings.h>

#include <esl/Stacktrace.h>

#include <stdexcept>

namespace jerry {
namespace builtin {
namespace http {
namespace basicauth {

Settings::Settings(const esl::object::Interface::Settings& settings) {
	for(const auto& setting : settings) {
		if(setting.first == "username") {
			username = setting.second;
		}
		else if(setting.first == "password") {
			password = setting.second;
		}
		else if(setting.first == "realmId") {
			realmId = setting.second;
		}
		else {
			throw esl::addStacktrace(std::runtime_error("Unknown parameter key=\"" + setting.first + "\" with value=\"" + setting.second + "\""));
		}
	}
}

const std::string& Settings::getUsername() const {
	return username;
}

const std::string& Settings::getPassword() const {
	return password;
}

const std::string& Settings::getRealmId() const {
	return realmId;
}

} /* namespace basicauth */
} /* namespace http */
} /* namespace builtin */
} /* namespace jerry */
