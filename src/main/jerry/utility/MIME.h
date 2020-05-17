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

#ifndef JERRY_UTILITY_MIME_H_
#define JERRY_UTILITY_MIME_H_

#include <string>

namespace jerry {
namespace utility {

struct MIME {
	MIME() = default;
	MIME(std::string mediaType, std::string subType) noexcept;

	std::string getContentType() noexcept;

	static MIME byFilename(const std::string& filename);

	// e.g. - application
	//      - audio
	//      - example
	//      - image
	//      - message
	//      - model
	//      - multipart
	//      - text
	//      - video
	std::string mediaType;

	// e.g. - jpeg
	std::string subType;
};

} /* namespace utility */
} /* namespace jerry */

#endif /* JERRY_UTILITY_MIME_H_ */
