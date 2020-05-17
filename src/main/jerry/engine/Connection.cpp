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

#include <jerry/engine/Connection.h>
#include <jerry/engine/Endpoint.h>
#include <jerry/engine/RequestContext.h>

namespace jerry {
namespace engine {

Connection::Connection(RequestContext& aRequestContext, esl::http::server::Connection& aBaseConnection)
: requestContext(aRequestContext),
  baseConnection(aBaseConnection)
{ }

bool Connection::sendResponse(std::unique_ptr<esl::http::server::ResponseBasicAuth> response) noexcept {
	addHeaders(*response);
	return baseConnection.sendResponse(std::move(response));
}

bool Connection::sendResponse(std::unique_ptr<esl::http::server::ResponseDynamic> response) noexcept {
	addHeaders(*response);
	return baseConnection.sendResponse(std::move(response));
}

bool Connection::sendResponse(std::unique_ptr<esl::http::server::ResponseFile> response) noexcept {
	addHeaders(*response);
	return baseConnection.sendResponse(std::move(response));
}

bool Connection::sendResponse(std::unique_ptr<esl::http::server::ResponseStatic> response) noexcept {
	addHeaders(*response);
	return baseConnection.sendResponse(std::move(response));
}

void Connection::addHeaders(esl::http::server::Response& response) {
	const std::map<std::string, std::string> headers = requestContext.getEngineContext().getEndpoint().getHeaders();

	for(const auto& header : headers) {
		response.addHeader(header.first, header.second);
	}
}

} /* namespace engine */
} /* namespace jerry */
