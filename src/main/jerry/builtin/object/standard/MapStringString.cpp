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

#include <jerry/builtin/object/standard/MapStringString.h>
#include <jerry/Logger.h>

#include <stdexcept>

namespace jerry {
namespace builtin {
namespace object {
namespace standard {

namespace {
std::map<std::string, std::string> makeValue(const std::vector<std::pair<std::string, std::string>>& settings) {
	std::map<std::string, std::string> value;

	for(const auto& setting : settings) {
		if(value.count(setting.first) > 0) {
			throw std::runtime_error("Multiple definition of attribute '" + setting.first + "'");
		}
		value.insert(std::make_pair(setting.first, setting.second));
	}

	return value;
}
} /* anonymous namespace */

std::unique_ptr<esl::object::Interface::Object> MapStringString::create(const std::vector<std::pair<std::string, std::string>>& settings) {
	return std::unique_ptr<esl::object::Interface::Object>(new MapStringString(settings));
}

MapStringString::MapStringString(const std::vector<std::pair<std::string, std::string>>& settings)
: Value<std::map<std::string, std::string>>(makeValue(settings))
{ }

} /* namespace standard */
} /* namespace object */
} /* namespace builtin */
} /* namespace jerry */
