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

#include <jerry/cgi/client/producer/RequestFile.h>
#if 0
namespace jerry {
namespace cgi {
namespace client {
namespace producer {

namespace {
RequestInfo createRequestInfo(std::size_t size) {
	return (size == esl::system::Interface::FileDescriptor::npos)
			? RequestInfo(true, false, size)
			: RequestInfo(size == 0, true, size);
}
}

RequestFile::RequestFile(const esl::http::client::RequestHandlerFile& requestHandler)
: ProducerFile(requestHandler.getFilename(), {{"read","true"}, {"write", "false"}, {"overwrite", "false"}}),
  requestInfo(createRequestInfo(getFileSize()))
{ }

const RequestInfo& RequestFile::getRequestInfo() const noexcept {
	return requestInfo;
}

} /* namespace producer */
} /* namespace client */
} /* namespace cgi */
} /* namespace jerry */
#endif
