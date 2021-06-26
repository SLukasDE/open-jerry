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

#ifndef JERRY_BUILTIN_SETTINGS_H_
#define JERRY_BUILTIN_SETTINGS_H_

//#include <esl/object/Settings.h>

#include <string>

namespace jerry {
namespace builtin {

class Settings2 {// : public esl::object::Settings {
protected:
	//using SetBoolMember = void (Settings::*)(bool);
	//void setParameterBool(const std::string& key, const std::string& value, SetBoolMember setBoolMember);
	static bool toBoolean(const std::string& value);

	//using SetIntegerMember = void (Settings::*)(int);
	//void setParameterInteger(const std::string& key, const std::string& value, SetIntegerMember setIntegerMember);
	static int toInteger(const std::string& value);
};

} /* namespace builtin */
} /* namespace jerry */

#endif /* JERRY_BUILTIN_SETTINGS_H_ */
