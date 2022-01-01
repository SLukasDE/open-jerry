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

#include <jerry/builtin/http/self/RequestHandler.h>
#include <jerry/Logger.h>

#include <esl/io/input/Closed.h>
#include <esl/io/output/String.h>
#include <esl/com/http/server/Request.h>
#include <esl/Stacktrace.h>

#include <stdexcept>
#include <string>

namespace jerry {
namespace builtin {
namespace http {
namespace self {

namespace {
Logger logger("jerry::builtin::http::self::RequestHandler");
} /* anonymous namespace */

std::unique_ptr<esl::com::http::server::requesthandler::Interface::RequestHandler> RequestHandler::createRequestHandler(const esl::object::Interface::Settings& settings) {
	for(const auto& setting : settings) {
		throw esl::addStacktrace(std::runtime_error("Unknown parameter key=\"" + setting.first + "\" with value=\"" + setting.second + "\""));
	}
	return std::unique_ptr<esl::com::http::server::requesthandler::Interface::RequestHandler>(new RequestHandler());
}

esl::io::Input RequestHandler::accept(esl::com::http::server::RequestContext& requestContext, esl::object::Interface::ObjectContext& objectContext) const {
	std::string content;
	content += "<!DOCTYPE html>\n";
	content += "<html>\n";
	content += "<head>\n";
	content += "<title>Self page</title>\n";
	content += "</head>\n";
	content += "<body>\n";
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
	content += "    <td>HOST</td>\n";
	content += "    <td>" + requestContext.getRequest().getHostName() + "</td>\n";
	content += "  </tr>\n";
	content += "  <tr>\n";
	content += "    <td>HOST_ADDRESS</td>\n";
	content += "    <td>" + requestContext.getRequest().getHostAddress() + "</td>\n";
	content += "  </tr>\n";
	content += "  <tr>\n";
	content += "    <td>HOST_PORT</td>\n";
	content += "    <td>" + std::to_string(requestContext.getRequest().getHostPort()) + "</td>\n";
	content += "  </tr>\n";
	content += "  <tr>\n";
	content += "    <td>REMOTE_ADDRESS</td>\n";
	content += "    <td>" + requestContext.getRequest().getRemoteAddress() + "</td>\n";
	content += "  </tr>\n";
	content += "  <tr>\n";
	content += "    <td>REMOTE_PORT</td>\n";
	content += "    <td>" + std::to_string(requestContext.getRequest().getRemotePort()) + "</td>\n";
	content += "  </tr>\n";
	content += "  <tr>\n";
	content += "    <td>HTTP_VERSION</td>\n";
	content += "    <td>" + requestContext.getRequest().getHTTPVersion() + "</td>\n";
	content += "  </tr>\n";
	content += "  <tr>\n";
	content += "    <td>USERNAME</td>\n";
	content += "    <td>" + requestContext.getRequest().getUsername() + "</td>\n";
	content += "  </tr>\n";
	content += "</table>\n";
	content += "</body>\n";
	content += "</html>\n";

	esl::com::http::server::Response response(200, esl::utility::MIME::textHtml);
	esl::io::Output output = esl::io::output::String::create(std::move(content));
	requestContext.getConnection().send(response, std::move(output));
	return esl::io::input::Closed::create();
}

} /* namespace self */
} /* namespace http */
} /* namespace builtin */
} /* namespace jerry */
