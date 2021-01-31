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

#include <jerry/utility/Base64.h>

namespace jerry {
namespace utility {

std::string Base64::toBase64(const std::string& str) {
	std::string base64Chars("ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/");
	std::string result;

	for(std::size_t i = 0; i < str.size(); i += 3) {
		int packed64 = (str[i] & 0xff) << 16;
		std::size_t num64Chars = 2;

		if(i + 1 < str.size()) {
			packed64 = packed64 + ((str[i+1] & 0xff) << 8);
			num64Chars = 3;
		}

		if(i + 2 < str.size()) {
			packed64 = packed64 + (str[i+2] & 0xff);
			num64Chars = 4;
		}

		for(std::size_t j = 0; j < 4; ++j) {
			if(j < num64Chars) {
				result += base64Chars[(packed64 >> (6 * (3 - j))) & 0x3f];
			}
			else {
				result += "=";
			}
		}
	}
	return result;
}

} /* namespace utility */
} /* namespace jerry */
