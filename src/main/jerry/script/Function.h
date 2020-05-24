/*
 * This file is part of Jerry application server.
 * Copyright (C) 2020 Sven Lukas
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

#ifndef JERRY_SCRIPT_FUNCTION_H_
#define JERRY_SCRIPT_FUNCTION_H_

#include <string>

namespace jerry {
namespace script {

class Function {
public:
	virtual ~Function() = default;

	/* Returns index of next argument needed.
	 * A return value of an argument index that has been set already indicates that method 'call' is ready to call.
	 *
	 * Before calling method 'call()' all necessary argument values have to be set to the function object.
	 * First call of 'setArgument(...)' must be 'setArgument(0, ...)' if there is more than zero arguments available.
	 * Typically 'setArgument(...)' returns 0 to indicate if method 'call' is ready to call.
	 *
	 * If no arguments are available 'call()' is ready to call directly.
	 */
	virtual std::size_t setArgument(std::size_t index, const std::string& value) = 0;

	virtual std::string call() = 0;
};

} /* namespace script */
} /* namespace jerry */

#endif /* JERRY_SCRIPT_FUNCTION_H_ */
