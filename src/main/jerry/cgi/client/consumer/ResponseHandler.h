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

#ifndef JERRY_CGI_CLIENT_CONSUMER_RESPONSEHANDLER_H_
#define JERRY_CGI_CLIENT_CONSUMER_RESPONSEHANDLER_H_
#if 0
#include <esl/system/Interface.h>
#include <esl/http/client/ResponseHandler.h>

#include <string>
#include <map>

namespace jerry {
namespace cgi {
namespace client {
namespace consumer {

class ResponseHandler : public esl::system::Interface::Consumer {
public:
	ResponseHandler(esl::http::client::ResponseHandler* responseHandler);

	std::size_t read(esl::system::Interface::FileDescriptor& fileDescriptor) override;

	unsigned short getStatusCode() const;

	std::map<std::string, std::string>& getHeaders() &;
	std::map<std::string, std::string>&& getHeaders() &&;

private:
	void addHeader(std::string line);
	bool process(const char* contentData, std::size_t contentSize);

	bool readHeader = true;
	mutable unsigned short statusCode = 500;
	std::map<std::string, std::string> headers;

	std::string buffer;

	esl::http::client::ResponseHandler* responseHandler;
};

} /* namespace consumer */
} /* namespace client */
} /* namespace cgi */
} /* namespace jerry */
#endif

#endif /* JERRY_CGI_CLIENT_CONSUMER_RESPONSEHANDLER_H_ */
