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

#ifndef JERRY_CGI_CLIENT_CONNECTION_H_
#define JERRY_CGI_CLIENT_CONNECTION_H_

#include <jerry/cgi/client/RequestInfo.h>

#include <esl/com/http/client/Interface.h>
#include <esl/com/http/client/Request.h>
#include <esl/com/http/client/Response.h>
//#include <esl/http/client/ResponseHandler.h>
#include <esl/system/Interface.h>
//#include <esl/system/process/Arguments.h>
//#include <esl/system/process/Environment.h>
#include <esl/utility/URL.h>

#include <string>
#include <vector>
#include <utility>
#include <memory>
#if 0
namespace jerry {
namespace cgi {
namespace client {

class Connection : public esl::http::client::Interface::Connection {
public:
	static std::unique_ptr<esl::http::client::Interface::Connection> create(const esl::utility::URL& hostUrl, const esl::object::Values<std::string>& values);

	static inline const char* getImplementation() {
		return "cgi4esl";
	}

	//Connection(std::string hostUrl, const esl::object::Values<std::string>& settings);
	Connection(const esl::utility::URL& hostUrl, esl::system::process::Arguments arguments, const esl::object::Values<std::string>& values);

	esl::http::client::Response send(esl::http::client::Request request) const override;

private:
	esl::http::client::Response send(const esl::http::client::Request& request, esl::http::client::ResponseHandler* responseHandler, esl::system::Interface::Producer& processProducer) const;

	void prepareRequest(esl::http::client::Response& response, const esl::http::client::Request& request, const std::string& method) const;

	esl::utility::URL hostUrl;
	std::string path;
	esl::system::process::Arguments arguments;
	std::string workingDir;

	/* e.g. jerry 1.0 */
	std::string serverSoftware;

	/* e.g. admin@jerry.org */
	std::string serverAdmin;

	std::string serverProtocol = "HTTP/1.1";

	std::string remoteHost;
	std::string remoteAddr;
	std::string remoteUser;
};

} /* namespace client */
} /* namespace cgi */
} /* namespace jerry */
#endif

#endif /* JERRY_CGI_CLIENT_CONNECTION_H_ */
