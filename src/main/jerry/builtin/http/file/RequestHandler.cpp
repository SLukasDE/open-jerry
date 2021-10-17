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
#include <jerry/builtin/http/file/Settings.h>
#include <jerry/utility/MIME.h>
#include <jerry/Logger.h>

#include <esl/com/http/server/Connection.h>
#include <esl/com/http/server/Response.h>
#include <esl/com/http/server/exception/StatusCode.h>
#include <esl/io/input/Closed.h>

#include <boost/filesystem/operations.hpp>
#include <boost/filesystem/path.hpp>

namespace jerry {
namespace builtin {
namespace http {
namespace file {

namespace {
Logger logger("jerry::builtin::http::file::RequestHandler");
} /* anonymous namespace */

esl::io::Input RequestHandler::createRequestHandler(esl::com::http::server::RequestContext& requestContext) {
	const Settings* settings = requestContext.findObject<Settings>();

	if(settings == nullptr) {
		logger.warn << "Settings object missing\n";
		throw esl::com::http::server::exception::StatusCode(500);
	}

	boost::filesystem::path path = settings->getPath();
	if(boost::filesystem::is_regular_file(path)) {
		esl::utility::MIME mime = utility::MIME::byFilename(settings->getPath());
		esl::com::http::server::Response response(settings->getHttpStatus(), mime);
		requestContext.getConnection().send(response, path);
		return esl::io::input::Closed::create();
	}

	logger.warn << "Path " << path << " is not a regular file\n";
	throw esl::com::http::server::exception::StatusCode(404);
}

std::unique_ptr<esl::object::Interface::Object> RequestHandler::createSettings(const esl::object::Interface::Settings& settings) {
	return std::unique_ptr<esl::object::Interface::Object>(new Settings(settings));
}

} /* namespace file */
} /* namespace http */
} /* namespace builtin */
} /* namespace jerry */
