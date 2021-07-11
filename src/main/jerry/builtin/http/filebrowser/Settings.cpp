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
#include <jerry/Logger.h>

#include <esl/Stacktrace.h>

#include <stdexcept>

namespace jerry {
namespace builtin {
namespace http {
namespace filebrowser {

namespace {
Logger logger("jerry::builtin::http::filebrowser::Settings");
}

Settings::Settings(const esl::object::Interface::Settings& settings) {
	for(const auto& setting : settings) {
		if(setting.first == "browsable") {
			if(setting.second == "true") {
				browsable = true;
			}
			else if(setting.second == "false") {
				browsable = false;
			}
			else {
				throw std::runtime_error("Unknown value \"" + setting.second + "\" for parameter key=\"" + setting.first + "\". Possible values are \"true\" or \"false\".");
			}
		}
		else if(setting.first == "path") {
			path = setting.second;
		}
		else if(setting.first == "default") {
			defaults.insert(setting.second);
		}
		else if(setting.first == "ignoreError") {
			if(setting.second == "true") {
				ignoreError = true;
			}
			else if(setting.second == "false") {
				ignoreError = false;
			}
			else {
				throw std::runtime_error("Unknown value \"" + setting.second + "\" for parameter key=\"" + setting.first + "\". Possible values are \"true\" or \"false\".");
			}
		}
		/*
		else if(setting.first == "accept-all") {
			setAcceptAll(key, value, &Settings::setShowException);
		}
		*/
		else {
			throw esl::addStacktrace(std::runtime_error("Unknown parameter key=\"" + setting.first + "\" with value=\"" + setting.second + "\""));
		}
	}
}

bool Settings::isBrowsable() const {
	return browsable;
}

const std::string& Settings::getPath() const {
	return path;
}

const std::set<std::string>& Settings::getDefaults() const {
	return defaults;
}

bool Settings::getIgnoreError() const {
	return ignoreError;
}

} /* namespace filebrowser */
} /* namespace http */
} /* namespace builtin */
} /* namespace jerry */
