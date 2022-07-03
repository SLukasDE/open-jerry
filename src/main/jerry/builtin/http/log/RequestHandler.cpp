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

#include <jerry/builtin/http/log/RequestHandler.h>
#include <jerry/utility/MIME.h>
#include <jerry/Logger.h>

#include <esl/com/http/server/Request.h>
#include <esl/io/Input.h>
#include <esl/system/Stacktrace.h>

#include <time.h>

#include <iostream>
#include <stdexcept>

namespace jerry {
namespace builtin {
namespace http {
namespace log {

namespace {
Logger logger("jerry::builtin::http::log::RequestHandler");
} /* anonymous namespace */

std::unique_ptr<esl::com::http::server::RequestHandler> RequestHandler::createRequestHandler(const std::vector<std::pair<std::string, std::string>>& settings) {
	return std::unique_ptr<esl::com::http::server::RequestHandler>(new RequestHandler(settings));
}

RequestHandler::RequestHandler(const std::vector<std::pair<std::string, std::string>>& settings) {
	for(const auto& setting : settings) {
		throw std::runtime_error("Unknown parameter key=\"" + setting.first + "\" with value=\"" + setting.second + "\"");
	}
}

esl::io::Input RequestHandler::accept(esl::com::http::server::RequestContext& requestContext) const {
	std::time_t timestamp;
	char timeStr[23];
	struct tm timeBuf;
	struct tm* timePtr;
	time(&timestamp);

	timePtr = localtime_r(&timestamp, &timeBuf);
	sprintf(timeStr, "$ %04d-%02d-%02d %02d:%02d:%02d ",
			timePtr->tm_year + 1900,
			timePtr->tm_mon  + 1,
			timePtr->tm_mday,
			timePtr->tm_hour,
			timePtr->tm_min,
			timePtr->tm_sec);
	std::cout << timeStr << "Request for hostname " << requestContext.getRequest().getHostName() << ": " << requestContext.getRequest().getMethod().toString() << " \"" << requestContext.getRequest().getPath() << "\" received from " << requestContext.getRequest().getRemoteAddress() << std::endl;
	return esl::io::Input();
}

} /* namespace log */
} /* namespace http */
} /* namespace builtin */
} /* namespace jerry */
