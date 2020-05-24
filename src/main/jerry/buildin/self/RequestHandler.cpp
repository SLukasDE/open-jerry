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

#include <jerry/buildin/self/RequestHandler.h>

#include <esl/http/server/ResponseDynamic.h>
#include <esl/http/server/Request.h>

#include <iostream>

namespace jerry {
namespace buildin {
namespace self {

std::unique_ptr<esl::http::server::requesthandler::Interface::RequestHandler> RequestHandler::create(esl::http::server::RequestContext& requestContext) {
	return std::unique_ptr<esl::http::server::requesthandler::Interface::RequestHandler>(new RequestHandler(requestContext));
}

RequestHandler::RequestHandler(esl::http::server::RequestContext& requestContext)
: esl::http::server::requesthandler::Interface::RequestHandler()
{
	std::string content(
			"<!DOCTYPE html>\n"
			"<html>\n"
			"<head>\n"
			"<title>Self page</title>\n"
			"</head>\n"
			"<body>\n");
	content += "<table style=\"width:100%\">\n";
	content += "  <tr>\n";
	content += "    <th>Key</th>\n";
	content += "    <th>Value</th>\n";
	content += "  </tr>\n";
	content += "  <tr>\n";
	content += "    <td>LOCAL_PATH</td>\n";
	content += "    <td>" + requestContext.getPath() + "</td>\n";
	content += "  </tr>\n";
	content += "  <tr>\n";
	content += "    <td>FULL_PATH</td>\n";
	content += "    <td>" + requestContext.getRequest().getPath() + "</td>\n";
	content += "  </tr>\n";
	content += "  <tr>\n";
	content += "    <td>METHOD</td>\n";
	content += "    <td>" + requestContext.getRequest().getMethod() + "</td>\n";
	content += "  </tr>\n";
	content += "  <tr>\n";
	content += "    <td>CLIENT_ADDRESS</td>\n";
	content += "    <td>" + requestContext.getRequest().getClientAddress() + "</td>\n";
	content += "  </tr>\n";
	content += "  <tr>\n";
	content += "    <td>DOMAIN</td>\n";
	content += "    <td>" + requestContext.getRequest().getDomain() + "</td>\n";
	content += "  </tr>\n";
	content += "  <tr>\n";
	content += "    <td>HTTP_VERSION</td>\n";
	content += "    <td>" + requestContext.getRequest().getHTTPVersion() + "</td>\n";
	content += "  </tr>\n";
	content += "  <tr>\n";
	content += "    <td>PORT</td>\n";
	content += "    <td>" + std::to_string(requestContext.getRequest().getPort()) + "</td>\n";
	content += "  </tr>\n";
	content += "  <tr>\n";
	content += "    <td>USERNAME</td>\n";
	content += "    <td>" + requestContext.getRequest().getUsername() + "</td>\n";
	content += "  </tr>\n";
	content += "</table>\n";
	content +=
			"</body>\n"
			"</html>\n";

	std::unique_ptr<esl::http::server::ResponseDynamic> response;
	response.reset(new esl::http::server::ResponseDynamic(200, esl::utility::MIME::textHtml, std::move(content)));
	requestContext.getConnection().sendResponse(std::move(response));
}

} /* namespace self */
} /* namespace buildin */
} /* namespace jerry */
