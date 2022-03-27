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

#include <jerry/builtin/object/standard/Int.h>
#include <jerry/Logger.h>

#include <stdexcept>

namespace jerry {
namespace builtin {
namespace object {
namespace standard {

namespace {
int makeValue(const std::vector<std::pair<std::string, std::string>>& settings) {
	int value = 0;
	bool hasValue = false;

	for(const auto& setting : settings) {
		if(setting.first == "value") {
			if(hasValue) {
				throw std::runtime_error("Multiple definition of attribute 'value'");
			}
			value = std::stoi(setting.second);
			hasValue = true;
		}
		else {
			throw std::runtime_error("Unknown parameter key=\"" + setting.first + "\" with value=\"" + setting.second + "\"");
		}
	}

	return value;
}
} /* anonymous namespace */

std::unique_ptr<esl::object::Interface::Object> Int::create(const std::vector<std::pair<std::string, std::string>>& settings) {
	return std::unique_ptr<esl::object::Interface::Object>(new Int(settings));
}

Int::Int(const std::vector<std::pair<std::string, std::string>>& settings)
: Value<int>(makeValue(settings))
{ }

} /* namespace standard */
} /* namespace object */
} /* namespace builtin */
} /* namespace jerry */
