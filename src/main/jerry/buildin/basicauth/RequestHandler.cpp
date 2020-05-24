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

#include <jerry/buildin/basicauth/RequestHandler.h>
#include <jerry/buildin/basicauth/Settings.h>
#include <jerry/Logger.h>

#include <esl/http/server/ResponseBasicAuth.h>
#include <esl/utility/MIME.h>

namespace jerry {
namespace buildin {
namespace basicauth {

namespace {
Logger logger("jerry::buildin::basicauth::RequestHandler");

const std::string PAGE_401(
		"<!DOCTYPE html>\n"
		"<html>\n"
		"<head>\n"
		"<title>401 - Unauthorized</title>\n"
		"</head>\n"
		"<body>\n"
		"<h1>401 - Unauthorized</h1>\n"
		"</body>\n"
		"</html>\n");
}

std::unique_ptr<esl::http::server::requesthandler::Interface::RequestHandler> RequestHandler::create(esl::http::server::RequestContext& requestContext) {
	const Settings* settings = dynamic_cast<Settings*>(requestContext.getObject(""));
	if(settings == nullptr) {
		return nullptr;
	}

	if(requestContext.getRequest().getUsername() == settings->getUsername() &&
			requestContext.getRequest().getPassword() == settings->getPassword()) {
		return nullptr;
	}

	return std::unique_ptr<esl::http::server::requesthandler::Interface::RequestHandler>(new RequestHandler(requestContext, settings->getRealmId()));
}

RequestHandler::RequestHandler(esl::http::server::RequestContext& requestContext, const std::string& realmId)
: esl::http::server::requesthandler::Interface::RequestHandler()
{
	std::unique_ptr<esl::http::server::ResponseBasicAuth> response;
	response.reset(new esl::http::server::ResponseBasicAuth(realmId, esl::utility::MIME::textHtml, PAGE_401.data(), PAGE_401.size()));
	requestContext.getConnection().sendResponse(std::move(response));
}
/*
bool RequestHandler::process(const char* contentData, std::size_t contentDataSize) {
	return false;
}
*/

} /* namespace basicauth */
} /* namespace buildin */
} /* namespace jerry */
