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

#include <jerry/builtin/Settings.h>

#include <esl/Stacktrace.h>

#include <stdexcept>

namespace jerry {
namespace builtin {
/*
void Settings::setParameterBool(const std::string& key, const std::string& value, SetBoolMember setBoolMember) {
	(this->*setBoolMember)(toBoolean(value));
}
*/
bool Settings2::toBoolean(const std::string& value) {
	bool b;

	if(value == "true") {
		b = true;
	}
	else if(value == "false") {
		b = false;
	}
	else {
		throw esl::addStacktrace(std::runtime_error("Cannot convert value=\"" + value + "\" to boolean"));
	}

	return b;
}
/*
void Settings::setParameterInteger(const std::string& key, const std::string& value, SetIntegerMember setIntegerMember) {
	(this->*setIntegerMember)(toInteger(value));
}
*/
int Settings2::toInteger(const std::string& value) {
	int i;

	try {
		i = std::stoi(value);
	}
	catch(...) {
		throw esl::addStacktrace(std::runtime_error("Cannot convert value=\"" + value + "\" to integer"));
	}

	return i;
}

} /* namespace builtin */
} /* namespace jerry */
