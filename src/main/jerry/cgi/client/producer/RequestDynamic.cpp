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

#include <jerry/cgi/client/producer/RequestDynamic.h>

namespace jerry {
namespace cgi {
namespace client {
namespace producer {
namespace {
RequestInfo createRequestInfo(const esl::http::client::RequestDynamic& request) {
	bool isEmpty(request.getSize() == 0);
	bool hasSize(isEmpty ? false : request.hasSize());
	std::size_t contentSize(hasSize ? 0 : request.getSize());

	return RequestInfo(isEmpty, hasSize, contentSize);
}
}

RequestDynamic::RequestDynamic(esl::http::client::RequestDynamic& aRequest)
: ProducerDynamic([&aRequest](char* buffer, std::size_t count) { return aRequest.produceData(buffer, count); }),
  request(aRequest),
  requestInfo(createRequestInfo(request))
{ }

const RequestInfo& RequestDynamic::getRequestInfo() const noexcept {
	return requestInfo;
}

} /* namespace producer */
} /* namespace client */
} /* namespace cgi */
} /* namespace jerry */
