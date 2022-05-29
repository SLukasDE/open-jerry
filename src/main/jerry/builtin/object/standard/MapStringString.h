/*
 * This file is part of Jerry application server.
 * Copyright (C) 2020-2022 Sven Lukas
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

#ifndef JERRY_BUILTIN_OBJECT_STANDARD_MAPSTRINGSTRING_H_
#define JERRY_BUILTIN_OBJECT_STANDARD_MAPSTRINGSTRING_H_

#include <esl/object/Value.h>
#include <esl/object/Interface.h>

#include <vector>
#include <map>
#include <string>
#include <utility>
#include <memory>

namespace jerry {
namespace builtin {
namespace object {
namespace standard {

class MapStringString : public esl::object::Value<std::map<std::string, std::string>> {
public:
	static inline const char* getImplementation() {
		return "std/map<string,string>";
	}

	static std::unique_ptr<esl::object::Interface::Object> create(const std::vector<std::pair<std::string, std::string>>& settings);

	MapStringString(const std::vector<std::pair<std::string, std::string>>& settings);
};

} /* namespace standard */
} /* namespace object */
} /* namespace builtin */
} /* namespace jerry */

#endif /* JERRY_BUILTIN_OBJECT_STANDARD_MAPSTRINGSTRING_H_ */