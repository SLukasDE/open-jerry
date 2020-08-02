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

#ifndef JERRY_CGI_CLIENT_REQUESTINFO_H_
#define JERRY_CGI_CLIENT_REQUESTINFO_H_

#include <string>

namespace jerry {
namespace cgi {
namespace client {

class RequestInfo {
public:
	RequestInfo(bool isEmpty, bool hasSize, std::size_t contentSize);

	bool isContentEmpty() const;
	bool hasContentSize() const;
	std::size_t getContentSize() const;

private:
	const bool isEmpty;
	const bool hasSize;
	const std::size_t contentSize;
};

} /* namespace client */
} /* namespace cgi */
} /* namespace jerry */

#endif /* JERRY_CGI_CLIENT_REQUESTINFO_H_ */
