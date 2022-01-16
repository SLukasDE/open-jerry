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

#include <jerry/builtin/http/basicauth/response/RequestHandler.h>
#include <jerry/Logger.h>

#include <esl/com/http/server/Response.h>
#include <esl/com/http/server/Request.h>
#include <esl/com/http/server/exception/StatusCode.h>
#include <esl/io/output/Memory.h>
#include <esl/io/input/Closed.h>
#include <esl/io/Input.h>
#include <esl/utility/MIME.h>
#include <esl/object/Properties.h>

#include <stdexcept>

namespace jerry {
namespace builtin {
namespace http {
namespace basicauth {
namespace response {

namespace {
Logger logger("jerry::builtin::http::basicauth::response::RequestHandler");

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

std::unique_ptr<esl::com::http::server::requesthandler::Interface::RequestHandler> RequestHandler::createRequestHandler(const esl::module::Interface::Settings& settings) {
	return std::unique_ptr<esl::com::http::server::requesthandler::Interface::RequestHandler>(new RequestHandler(settings));
}

RequestHandler::RequestHandler(const esl::module::Interface::Settings& settings) {
	for(const auto& setting : settings) {
		if(setting.first == "realm-id") {
			if(!realmId.empty()) {
				throw std::runtime_error("Multiple definition of attribute 'realm-id'");
			}
			realmId = setting.second;
			if(realmId.empty()) {
				throw std::runtime_error("Invalid value \"\" for attribute 'realm-id'");
			}
		}
		else {
			throw std::runtime_error("Unknown parameter key=\"" + setting.first + "\" with value=\"" + setting.second + "\"");
		}
	}

	if(realmId.empty()) {
		throw std::runtime_error("Definition of attribute 'realm-id' is missing");
	}
}

esl::io::Input RequestHandler::accept(esl::com::http::server::RequestContext& requestContext) const {
	esl::object::Properties* authProperties = requestContext.getObjectContext().findObject<esl::object::Properties>("auth");
	if(authProperties && authProperties->hasValue("identified")) {
		return esl::io::Input();
	}

	esl::com::http::server::Response response(401, esl::utility::MIME(esl::utility::MIME::textHtml), realmId);
	esl::io::Output output = esl::io::output::Memory::create(PAGE_401.data(), PAGE_401.size());
	requestContext.getConnection().send(response, std::move(output));
	return esl::io::input::Closed::create();
}

} /* namespace response */
} /* namespace basicauth */
} /* namespace http */
} /* namespace builtin */
} /* namespace jerry */
