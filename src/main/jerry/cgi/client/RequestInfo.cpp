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

#include <jerry/cgi/client/RequestInfo.h>

namespace jerry {
namespace cgi {
namespace client {

RequestInfo::RequestInfo(bool aIsEmpty, bool aHasSize, std::size_t aContentSize)
: isEmpty(aIsEmpty),
  hasSize(aHasSize),
  contentSize(aContentSize)
{ }

bool RequestInfo::isContentEmpty() const {
	return isEmpty;
}

bool RequestInfo::hasContentSize() const {
	return hasSize;
}

std::size_t RequestInfo::getContentSize() const {
	return contentSize;
}

} /* namespace client */
} /* namespace cgi */
} /* namespace jerry */
