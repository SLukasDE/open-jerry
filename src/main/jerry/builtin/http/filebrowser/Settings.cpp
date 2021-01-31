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

#include <jerry/builtin/http/filebrowser/Settings.h>

#include <esl/Stacktrace.h>

#include <stdexcept>

namespace jerry {
namespace builtin {
namespace http {
namespace filebrowser {

std::unique_ptr<esl::object::Interface::Object> Settings::create() {
	return std::unique_ptr<esl::object::Interface::Object>(new Settings);
}

void Settings::addSetting(const std::string& key, const std::string& value) {
	if(key == "browsable") {
		if(value == "true") {
			setBrowsable(true);
		}
		else if(value == "false") {
			setBrowsable(false);
		}
		else {
			throw std::runtime_error("Unknown value \"" + value + "\" for parameter key=\"" + key + "\". Possible values are \"true\" or \"false\".");
		}
	}
	else if(key == "path") {
		setPath(value);
	}
	else if(key == "http-status") {
		setHttpStatus(toInteger(value));
	}
	else if(key == "default") {
		addDefault(value);
	}
	/*
	else if(key == "accept-all") {
		setAcceptAll(key, value, &Settings::setShowException);
	}
	*/
	else {
		throw esl::addStacktrace(std::runtime_error("Unknown parameter key=\"" + key + "\" with value=\"" + value + "\""));
	}
}

void Settings::setBrowsable(bool aBrowsable) {
	browsable = aBrowsable;
}

bool Settings::isBrowsable() const {
	return browsable;
}

void Settings::setPath(const std::string& aPath) {
	path = aPath;
}

const std::string& Settings::getPath() const {
	return path;
}

void Settings::addDefault(const std::string& file) {
	defaults.insert(file);
}

const std::set<std::string>& Settings::getDefaults() const {
	return defaults;
}

void Settings::setHttpStatus(int aHttpStatus) {
	httpStatus = aHttpStatus;
}

const int Settings::getHttpStatus() const {
	return httpStatus;
}

} /* namespace filebrowser */
} /* namespace http */
} /* namespace builtin */
} /* namespace jerry */
