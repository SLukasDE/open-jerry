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

#include <jerry/builtin/object/standard/VectorPairStringString.h>

namespace jerry {
namespace builtin {
namespace object {
namespace standard {

std::unique_ptr<esl::object::Object> VectorPairStringString::create(const std::vector<std::pair<std::string, std::string>>& settings) {
	return std::unique_ptr<esl::object::Object>(new VectorPairStringString(settings));
}

VectorPairStringString::VectorPairStringString(const std::vector<std::pair<std::string, std::string>>& settings)
: Value<std::vector<std::pair<std::string, std::string>>>(settings)
{ }

} /* namespace standard */
} /* namespace object */
} /* namespace builtin */
} /* namespace jerry */
