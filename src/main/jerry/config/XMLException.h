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

#ifndef JERRY_CONFIG_XMLEXCEPTION_H_
#define JERRY_CONFIG_XMLEXCEPTION_H_

#include <jerry/config/Config.h>

#include <tinyxml2/tinyxml2.h>

#include <string>
#include <exception>

namespace jerry {
namespace config {

class XMLException : public std::exception {
public:
	explicit XMLException(const std::string& fileName, int lineNo, tinyxml2::XMLError xmlError);
	explicit XMLException(const std::string& fileName, int lineNo, const std::string& message);
	explicit XMLException(const Config& config, tinyxml2::XMLError xmlError);
	explicit XMLException(const Config& config, const std::string& message);

	const char* what() const noexcept override;

private:
	const std::string fullMessage;
};

} /* namespace config */
} /* namespace jerry */

#endif /* JERRY_CONFIG_XMLEXCEPTION_H_ */
