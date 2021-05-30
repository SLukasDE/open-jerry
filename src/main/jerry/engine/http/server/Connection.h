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

#ifndef JERRY_ENGINE_HTTP_SERVER_CONNECTION_H_
#define JERRY_ENGINE_HTTP_SERVER_CONNECTION_H_

#include <esl/http/server/Connection.h>
#include <esl/http/server/Response.h>

#include <boost/filesystem.hpp>

namespace jerry {
namespace engine {
namespace http {
namespace server {

class RequestContext;

class Connection : public esl::http::server::Connection {
public:
	Connection(RequestContext& requestContext, esl::http::server::Connection& baseConnection);

	bool sendResponse(const esl::http::server::Response& response, esl::io::Output output) noexcept override;
	bool sendResponse(const esl::http::server::Response& response, boost::filesystem::path path) noexcept override;

private:
	RequestContext& requestContext;
	esl::http::server::Connection& baseConnection;

	void addHeaders(esl::http::server::Response& response);
};

} /* namespace server */
} /* namespace http */
} /* namespace engine */
} /* namespace jerry */

#endif /* JERRY_ENGINE_HTTP_SERVER_CONNECTION_H_ */
