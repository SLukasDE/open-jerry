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

#include <jerry/cgi/client/producer/RequestStatic.h>
#if 0
namespace jerry {
namespace cgi {
namespace client {
namespace producer {

RequestStatic::RequestStatic(const esl::http::client::RequestHandlerStatic& requestHandler)
: ProducerStatic(requestHandler.getData(), requestHandler.getSize()),
  requestInfo(requestHandler.getSize() == 0, true, requestHandler.getSize())
{ }

const RequestInfo& RequestStatic::getRequestInfo() const noexcept {
	return requestInfo;
}

} /* namespace producer */
} /* namespace client */
} /* namespace cgi */
} /* namespace jerry */
#endif
