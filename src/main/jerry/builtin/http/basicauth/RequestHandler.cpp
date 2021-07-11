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

#include <jerry/builtin/http/basicauth/RequestHandler.h>
#include <jerry/builtin/http/basicauth/Settings.h>
#include <jerry/Logger.h>

#include <esl/com/http/server/Response.h>
#include <esl/com/http/server/Request.h>
#include <esl/com/http/server/exception/StatusCode.h>
#include <esl/io/output/Memory.h>
#include <esl/io/input/Closed.h>
#include <esl/utility/MIME.h>

#include <string>

namespace jerry {
namespace builtin {
namespace http {
namespace basicauth {

namespace {
Logger logger("jerry::builtin::basicauth::RequestHandler");

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
} /* anonymous namespace */

esl::io::Input RequestHandler::createRequestHandler(esl::com::http::server::RequestContext& requestContext) {
	const Settings* settings = requestContext.findObject<Settings>();

	if(settings == nullptr) {
		logger.warn << "Settings object missing\n";
		throw esl::com::http::server::exception::StatusCode(500);
	}

	if(requestContext.getRequest().getUsername() == settings->getUsername() &&
			requestContext.getRequest().getPassword() == settings->getPassword()) {
		return esl::io::Input();
	}

	esl::com::http::server::Response response(401, esl::utility::MIME(esl::utility::MIME::textHtml), settings->getRealmId());
	esl::io::Output output = esl::io::output::Memory::create(PAGE_401.data(), PAGE_401.size());
	requestContext.getConnection().send(response, std::move(output));
	return esl::io::input::Closed::create();
}

std::unique_ptr<esl::object::Interface::Object> RequestHandler::createSettings(const esl::object::Interface::Settings& settings) {
	return std::unique_ptr<esl::object::Interface::Object>(new Settings(settings));
}

} /* namespace basicauth */
} /* namespace http */
} /* namespace builtin */
} /* namespace jerry */
