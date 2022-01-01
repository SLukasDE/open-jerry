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

#include <jerry/script/builtin/Base64.h>
#include <jerry/utility/Base64.h>

#include <stdexcept>

namespace jerry {
namespace script {
namespace builtin {

std::size_t Base64::setArgument(std::size_t index, const std::string& value) {
	if(index == 0) {
		argument = value;
	}
	else {
		throw std::runtime_error("calling Base64::setArgument with invalid index=" + std::to_string(index));
	}
	return 0;
}

std::string Base64::call() {
	return utility::Base64::toBase64(argument);
}

} /* namespace builtin */
} /* namespace script */
} /* namespace jerry */
