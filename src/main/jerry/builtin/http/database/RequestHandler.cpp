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

#include <jerry/builtin/http/database/RequestHandler.h>
#include <jerry/builtin/http/database/Settings.h>
#include <jerry/Logger.h>

#include <esl/com/http/server/Response.h>
#include <esl/com/http/server/Request.h>
#include <esl/com/http/server/exception/StatusCode.h>
//#include <esl/io/output/Memory.h>
#include <esl/io/output/String.h>
#include <esl/io/input/Closed.h>
#include <esl/utility/MIME.h>

#include <string>

namespace jerry {
namespace builtin {
namespace http {
namespace database {

namespace {
Logger logger("jerry::builtin::database::RequestHandler");

const std::string PAGE_200(
		"<!DOCTYPE html>\n"
		"<html>\n"
		"<head>\n"
		"<title>Database connection test</title>\n"
		"</head>\n"
		"<body>\n"
		"<h1>OK</h1>\n"
		"</body>\n"
		"</html>\n");

const std::string PAGE_500(
		"<!DOCTYPE html>\n"
		"<html>\n"
		"<head>\n"
		"<title>Database connection test</title>\n"
		"</head>\n"
		"<body>\n"
		"<h1>Failure</h1>\n"
		"</body>\n"
		"</html>\n");
} /* anonymous namespace */

esl::io::Input RequestHandler::createRequestHandler(esl::com::http::server::RequestContext& requestContext) {
	const Settings* settings = requestContext.findObject<Settings>();

	if(settings == nullptr) {
		logger.warn << "Settings object missing\n";
		throw esl::com::http::server::exception::StatusCode(500);
	}

	if(settings->check()) {
		esl::com::http::server::Response response(200, esl::utility::MIME(esl::utility::MIME::textHtml));
		//esl::io::Output output = esl::io::output::Memory::create(PAGE_200.data(), PAGE_200.size());
		esl::io::Output output = esl::io::output::String::create(PAGE_200);
		requestContext.getConnection().send(response, std::move(output));
		logger.debug << "OK\n";
	}
	else {
		esl::com::http::server::Response response(500, esl::utility::MIME(esl::utility::MIME::textHtml));
		//esl::io::Output output = esl::io::output::Memory::create(PAGE_500.data(), PAGE_500.size());
		esl::io::Output output = esl::io::output::String::create(PAGE_500);
		requestContext.getConnection().send(response, std::move(output));
		logger.debug << "Failure\n";
	}

	return esl::io::input::Closed::create();
}

std::unique_ptr<esl::object::Interface::Object> RequestHandler::createSettings(const esl::object::Interface::Settings& settings) {
	return std::unique_ptr<esl::object::Interface::Object>(new Settings(settings));
}

} /* namespace database */
} /* namespace http */
} /* namespace builtin */
} /* namespace jerry */
