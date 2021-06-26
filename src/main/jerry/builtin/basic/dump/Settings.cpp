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

#include <jerry/builtin/basic/dump/Settings.h>

#include <esl/Stacktrace.h>

#include <stdexcept>

namespace jerry {
namespace builtin {
namespace basic {
namespace dump {

Settings::Settings(const esl::object::Interface::Settings& settings) {
	for(const auto& setting : settings) {
		if(setting.first == "show-context") {
			if(setting.second == "true") {
				showContext = true;
			}
			else if(setting.second == "false") {
				showContext = false;
			}
			else {
				throw std::runtime_error("Unknown value \"" + setting.second + "\" for parameter key=\"" + setting.first + "\". Possible values are \"true\" or \"false\".");
			}
		}
		else if(setting.first == "show-content") {
			if(setting.second == "true") {
				showContent = true;
			}
			else if(setting.second == "false") {
				showContent = false;
			}
			else {
				throw std::runtime_error("Unknown value \"" + setting.second + "\" for parameter key=\"" + setting.first + "\". Possible values are \"true\" or \"false\".");
			}
		}
		else if(setting.first == "notifier") {
			notifiers.insert(setting.second);
		}
		else {
			throw esl::addStacktrace(std::runtime_error("Unknown parameter key=\"" + setting.first + "\" with value=\"" + setting.second + "\""));
		}
	}
}

bool Settings::getShowContext() const noexcept {
	return showContext;
}

bool Settings::getShowContent() const noexcept {
	return showContent;
}

const std::set<std::string>& Settings::getNotifiers() const noexcept {
	return notifiers;
}

} /* namespace dump */
} /* namespace basic */
} /* namespace builtin */
} /* namespace jerry */
