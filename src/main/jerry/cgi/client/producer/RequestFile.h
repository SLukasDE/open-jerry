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

#ifndef JERRY_CGI_CLIENT_PRODUCER_REQUESTFILE_H_
#define JERRY_CGI_CLIENT_PRODUCER_REQUESTFILE_H_

#include <jerry/cgi/client/RequestInfo.h>

#include <esl/http/client/RequestFile.h>
#include <esl/system/process/ProducerFile.h>

#include <string>

namespace jerry {
namespace cgi {
namespace client {
namespace producer {

class RequestFile : public esl::system::process::ProducerFile {
public:
	RequestFile(const esl::http::client::RequestFile& request);

	const RequestInfo& getRequestInfo() const noexcept;

private:
	RequestInfo requestInfo;
};

} /* namespace producer */
} /* namespace client */
} /* namespace cgi */
} /* namespace jerry */

#endif /* JERRY_CGI_CLIENT_PRODUCER_REQUESTFILE_H_ */
