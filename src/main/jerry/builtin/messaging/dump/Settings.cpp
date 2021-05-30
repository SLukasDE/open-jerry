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

#include <jerry/builtin/messaging/dump/Settings.h>

#include <esl/Stacktrace.h>

#include <stdexcept>

namespace jerry {
namespace builtin {
namespace messaging {
namespace dump {

std::unique_ptr<esl::object::Interface::Object> Settings::create() {
	return std::unique_ptr<esl::object::Interface::Object>(new Settings);
}

void Settings::addSetting(const std::string& key, const std::string& value) {
	if(key == "show-context") {
		setShowContent(toBoolean(value));
	}
	else if(key == "show-content") {
		setShowContent(toBoolean(value));
	}
	else if(key == "notifier") {
		notifiers.insert(value);
	}
	else {
		throw esl::addStacktrace(std::runtime_error("Unknown parameter key=\"" + key + "\" with value=\"" + value + "\""));
	}
}

void Settings::setShowContext(bool aShowContext) noexcept {
	showContext = aShowContext;
}

bool Settings::getShowContext() const noexcept {
	return showContext;
}

void Settings::setShowContent(bool aShowContent) noexcept {
	showContent = aShowContent;
}

bool Settings::getShowContent() const noexcept {
	return showContent;
}

const std::set<std::string>& Settings::getNotifiers() const noexcept {
	return notifiers;
}

} /* namespace dump */
} /* namespace messaging */
} /* namespace builtin */
} /* namespace jerry */
