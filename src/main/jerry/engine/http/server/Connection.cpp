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

#include <jerry/engine/http/server/Connection.h>

namespace jerry {
namespace engine {
namespace http {
namespace server {

Connection::Connection(esl::com::http::server::Connection& aBaseConnection, const Context& aHeadersContext)
: baseConnection(aBaseConnection),
  headersContext(aHeadersContext)
{ }

bool Connection::send(const esl::com::http::server::Response& aResponse, esl::io::Output output) {
	esl::com::http::server::Response response(aResponse);
	addHeaders(response);
	return baseConnection.send(response, std::move(output));
}

bool Connection::send(const esl::com::http::server::Response& aResponse, boost::filesystem::path path) {
	esl::com::http::server::Response response(aResponse);
	addHeaders(response);
	return baseConnection.send(response, std::move(path));
}

void Connection::addHeaders(esl::com::http::server::Response& response) {
	const std::map<std::string, std::string>& headers = headersContext.getEffectiveHeaders();

	for(const auto& header : headers) {
		response.addHeader(header.first, header.second);
	}
}

} /* namespace server */
} /* namespace http */
} /* namespace engine */
} /* namespace jerry */
