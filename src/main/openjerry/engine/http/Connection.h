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

#ifndef OPENJERRY_ENGINE_HTTP_CONNECTION_H_
#define OPENJERRY_ENGINE_HTTP_CONNECTION_H_

#include <openjerry/engine/http/Context.h>

#include <esl/com/http/server/Connection.h>
#include <esl/com/http/server/Response.h>
#include <esl/io/Output.h>

namespace openjerry {
namespace engine {
namespace http {


class RequestContext;

class Connection : public esl::com::http::server::Connection {
public:
	Connection(RequestContext& requestContext, esl::com::http::server::Connection& baseConnection);

	bool send(const esl::com::http::server::Response& response, esl::io::Output output) override;
	bool sendFile(const esl::com::http::server::Response& response, const std::string& path) override;

private:
	RequestContext& requestContext;
	esl::com::http::server::Connection& baseConnection;

	void addHeaders(esl::com::http::server::Response& response);
};


} /* namespace http */
} /* namespace engine */
} /* namespace openjerry */

#endif /* OPENJERRY_ENGINE_HTTP_CONNECTION_H_ */
