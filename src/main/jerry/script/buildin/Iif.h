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

#ifndef JERRY_SCRIPT_BUILDIN_IIF_H_
#define JERRY_SCRIPT_BUILDIN_IIF_H_

#include <jerry/script/Function.h>

namespace jerry {
namespace script {
namespace buildin {

class Iif : public script::Function {
public:
	std::size_t setArgument(std::size_t index, const std::string& value) override;
	std::string call() override;
};

} /* namespace buildin */
} /* namespace script */
} /* namespace jerry */

#endif /* JERRY_SCRIPT_BUILDIN_IIF_H_ */
