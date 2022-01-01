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

#include <jerry/builtin/Properties.h>

#include <esl/object/Properties.h>

namespace jerry {
namespace builtin {

std::unique_ptr<esl::object::Interface::Object> Properties::createSettings(const esl::object::Interface::Settings& settings) {
	return std::unique_ptr<esl::object::Interface::Object>(new esl::object::Properties(settings));
}

} /* namespace builtin */
} /* namespace jerry */
