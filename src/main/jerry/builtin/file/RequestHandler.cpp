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

#include <jerry/builtin/file/RequestHandler.h>
#include <jerry/utility/MIME.h>
#include <jerry/Logger.h>

#include <esl/http/server/Connection.h>
#include <esl/http/server/ResponseFile.h>
#include <esl/Stacktrace.h>
#include <esl/logging/Level.h>

#include <memory>

namespace jerry {
namespace builtin {
namespace file {

namespace {
Logger logger("jerry::builtin::file::RequestHandler");
}

std::unique_ptr<esl::http::server::requesthandler::Interface::RequestHandler> RequestHandler::create(esl::http::server::RequestContext& requestContext) {
	const Settings* settings = dynamic_cast<Settings*>(requestContext.findObject(""));
	if(settings == nullptr) {
		return nullptr;
	}

	return std::unique_ptr<esl::http::server::requesthandler::Interface::RequestHandler>(new RequestHandler(requestContext, *settings));
}

RequestHandler::RequestHandler(esl::http::server::RequestContext& requestContext, const Settings& settings)
: esl::http::server::requesthandler::Interface::RequestHandler()
{
	esl::utility::MIME mime = utility::MIME::byFilename(settings.getPath());
	std::unique_ptr<esl::http::server::ResponseFile> response(new esl::http::server::ResponseFile(settings.getHttpStatus(), mime, settings.getPath()));
	requestContext.getConnection().sendResponse(std::move(response));
}

} /* namespace file */
} /* namespace builtin */
} /* namespace jerry */
