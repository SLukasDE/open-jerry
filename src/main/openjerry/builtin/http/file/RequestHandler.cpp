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

#include <openjerry/builtin/http/file/RequestHandler.h>
#include <openjerry/Logger.h>
#include <openjerry/utility/MIME.h>

#include <esl/com/http/server/Connection.h>
#include <esl/com/http/server/Response.h>
#include <esl/com/http/server/exception/StatusCode.h>
#include <esl/io/input/Closed.h>
#include <esl/system/Stacktrace.h>
#include <esl/utility/MIME.h>

#include <filesystem>
#include <stdexcept>

namespace openjerry {
namespace builtin {
namespace http {
namespace file {

namespace {
Logger logger("openjerry::builtin::http::file::RequestHandler");
} /* anonymous namespace */

std::unique_ptr<esl::com::http::server::RequestHandler> RequestHandler::createRequestHandler(const std::vector<std::pair<std::string, std::string>>& settings) {
	return std::unique_ptr<esl::com::http::server::RequestHandler>(new RequestHandler(settings));
}

RequestHandler::RequestHandler(const std::vector<std::pair<std::string, std::string>>& settings) {
	for(const auto& setting : settings) {
		if(setting.first == "path") {
			path = setting.second;
		}
		else if(setting.first == "http-status") {
			try {
				httpStatus = std::stoi(setting.second);
			}
			catch(...) {
				throw std::runtime_error("Invalid value \"" + setting.second + "\" for parameter key=\"" + setting.first + "\". Value must be an integer");
			}
		}
		else {
			throw std::runtime_error("Unknown parameter key=\"" + setting.first + "\" with value=\"" + setting.second + "\"");
		}
	}
}

esl::io::Input RequestHandler::accept(esl::com::http::server::RequestContext& requestContext) const {
	if(!std::filesystem::is_regular_file(path)) {
		logger.warn << "Path \"" << path << "\" is not a regular file\n";
		throw esl::com::http::server::exception::StatusCode(404);
	}

    esl::utility::MIME mime = openjerry::utility::MIME::byFilename(path);
	esl::com::http::server::Response response(httpStatus, mime);
	requestContext.getConnection().sendFile(response, path);
	return esl::io::input::Closed::create();
}

} /* namespace file */
} /* namespace http */
} /* namespace builtin */
} /* namespace openjerry */
