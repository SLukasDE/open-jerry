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

#include <jerry/builtin/http/file/RequestHandler.h>
#include <jerry/utility/MIME.h>
#include <jerry/Logger.h>

#include <esl/com/http/server/Connection.h>
#include <esl/Stacktrace.h>
#include <esl/logging/Level.h>

#include <memory>

namespace jerry {
namespace builtin {
namespace http {
namespace file {

namespace {
Logger logger("jerry::builtin::http::file::RequestHandler");
}

esl::io::Input RequestHandler::createRequestHandler(esl::com::http::server::RequestContext& requestContext) {
	const Settings* settings = requestContext.findObject<Settings>("");
	if(settings == nullptr) {
		return esl::io::Input();
	}

	return esl::io::Input(std::unique_ptr<esl::io::Consumer>(new RequestHandler(requestContext, *settings)));
}

std::unique_ptr<esl::object::Interface::Object> RequestHandler::createSettings(const esl::object::Interface::Settings& settings) {
	return std::unique_ptr<esl::object::Interface::Object>(new Settings(settings));
}

RequestHandler::RequestHandler(esl::com::http::server::RequestContext& requestContext, const Settings& settings)
{
	esl::utility::MIME mime = utility::MIME::byFilename(settings.getPath());
	esl::com::http::server::Response response(settings.getHttpStatus(), mime);
	requestContext.getConnection().send(response, settings.getPath());
}

bool RequestHandler::consume(esl::io::Reader& reader) {
	return false;
}

} /* namespace file */
} /* namespace http */
} /* namespace builtin */
} /* namespace jerry */
