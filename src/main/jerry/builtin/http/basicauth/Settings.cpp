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

std::unique_ptr<esl::object::Interface::Object> Settings::create() {
	return std::unique_ptr<esl::object::Interface::Object>(new Settings);
}

void Settings::addSetting(const std::string& key, const std::string& value) {
	if(key == "username") {
		setUsername(value);
	}
	else if(key == "password") {
		setPassword(value);
	}
	else if(key == "realmId") {
		setRealmId(value);
	}
	else {
		throw esl::addStacktrace(std::runtime_error("Unknown parameter key=\"" + key + "\" with value=\"" + value + "\""));
	}
}

void Settings::setUsername(std::string aUsername) {
	username = std::move(aUsername);
}

const std::string& Settings::getUsername() const {
	return username;
}

void Settings::setPassword(std::string aPassword) {
	password = std::move(aPassword);
}

const std::string& Settings::getPassword() const {
	return password;
}

void Settings::setRealmId(std::string aRealmId) {
	realmId = std::move(aRealmId);
}

const std::string& Settings::getRealmId() const {
	return realmId;
}

} /* namespace basicauth */
} /* namespace http */
} /* namespace builtin */
} /* namespace jerry */
