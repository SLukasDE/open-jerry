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

#ifndef JERRY_CONFIG_HTTP_SERVER_H_
#define JERRY_CONFIG_HTTP_SERVER_H_

#include <jerry/config/Setting.h>

#include <tinyxml2/tinyxml2.h>

//#include <cstdint>
#include <string>
#include <vector>
#include <ostream>

namespace jerry {
namespace config {
namespace http {

struct Server {
	Server(const tinyxml2::XMLElement& element);

	void save(std::ostream& oStream, std::size_t spaces) const;

	std::string id;
	std::string implementation;
	bool isHttps = false;
	std::vector<Setting> settings;
};

} /* namespace http */
} /* namespace config */
} /* namespace jerry */

#endif /* JERRY_CONFIG_HTTP_SERVER_H_ */
