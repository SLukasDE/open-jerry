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

#ifndef OPENJERRY_HTTP_STATUSCODE_H_
#define OPENJERRY_HTTP_STATUSCODE_H_

#include <string>

namespace openjerry {
namespace http {

class StatusCode final {
public:
	StatusCode() = delete;

	static std::string getMessage(short int statusCode);
};

} /* namespace http */
} /* namespace openjerry */

#endif /* OPENJERRY_HTTP_STATUSCODE_H_ */
