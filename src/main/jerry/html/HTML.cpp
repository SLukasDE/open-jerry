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

#include <jerry/html/HTML.h>

#include <sstream>

namespace jerry {
namespace html {

std::string toHTML(const std::string& str) {
	std::stringstream result;

	for(const auto c :str) {
		switch(c) {
		case '\n':
			result << "<br>\n";
			break;
		case '"':
			result << "&quot;";
			break;
		case '&':
			result << "&amp;";
			break;
		case '<':
			result << "&lt;";
			break;
		case '>':
			result << "&gt;";
			break;
		case ' ':
			result << "&nbsp;";
			break;
		default:
			result << c;
			break;
		}
	}

	return result.str();
}

} /* namespace html */
} /* namespace jerry */
