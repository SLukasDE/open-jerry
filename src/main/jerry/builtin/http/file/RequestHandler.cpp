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
#include <esl/com/http/server/Response.h>
#include <esl/com/http/server/exception/StatusCode.h>
#include <esl/io/input/Closed.h>
#include <esl/Stacktrace.h>

#include <boost/filesystem/operations.hpp>
#include <boost/filesystem/path.hpp>

#include <stdexcept>

namespace jerry {
namespace builtin {
namespace http {
namespace file {

namespace {
Logger logger("jerry::builtin::http::file::RequestHandler");
} /* anonymous namespace */

std::unique_ptr<esl::com::http::server::requesthandler::Interface::RequestHandler> RequestHandler::createRequestHandler(const esl::object::Interface::Settings& settings) {
	return std::unique_ptr<esl::com::http::server::requesthandler::Interface::RequestHandler>(new RequestHandler(settings));
}

RequestHandler::RequestHandler(const esl::object::Interface::Settings& settings) {
	for(const auto& setting : settings) {
		if(setting.first == "path") {
			path = setting.second;
		}
		else if(setting.first == "http-status") {
			try {
				httpStatus = std::stoi(setting.second);
			}
			catch(...) {
				throw esl::addStacktrace(std::runtime_error("Invalid value \"" + setting.second + "\" for parameter key=\"" + setting.first + "\". Value must be an integer"));
			}
		}
		else {
			throw esl::addStacktrace(std::runtime_error("Unknown parameter key=\"" + setting.first + "\" with value=\"" + setting.second + "\""));
		}
	}
}

esl::io::Input RequestHandler::accept(esl::com::http::server::RequestContext& requestContext, esl::object::Interface::ObjectContext& objectContext) const {
	if(!boost::filesystem::is_regular_file(path)) {
		logger.warn << "Path \"" << path << "\" is not a regular file\n";
		throw esl::com::http::server::exception::StatusCode(404);
	}

	esl::utility::MIME mime = utility::MIME::byFilename(path);
	esl::com::http::server::Response response(httpStatus, mime);
	requestContext.getConnection().send(response, boost::filesystem::path(path));
	return esl::io::input::Closed::create();
}

} /* namespace file */
} /* namespace http */
} /* namespace builtin */
} /* namespace jerry */
