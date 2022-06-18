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

#include <jerry/builtin/http/dump/RequestHandler.h>
#include <jerry/utility/MIME.h>
#include <jerry/Logger.h>

#include <esl/com/http/server/Request.h>
#include <esl/io/Input.h>
#include <esl/stacktrace/Stacktrace.h>

#include <stdexcept>

namespace jerry {
namespace builtin {
namespace http {
namespace dump {

namespace {
Logger logger("jerry::builtin::http::dump::RequestHandler");
} /* anonymous namespace */

std::unique_ptr<esl::com::http::server::requesthandler::Interface::RequestHandler> RequestHandler::createRequestHandler(const std::vector<std::pair<std::string, std::string>>& settings) {
	return std::unique_ptr<esl::com::http::server::requesthandler::Interface::RequestHandler>(new RequestHandler(settings));
}

RequestHandler::RequestHandler(const std::vector<std::pair<std::string, std::string>>& settings) {
	for(const auto& setting : settings) {
		throw std::runtime_error("Unknown parameter key=\"" + setting.first + "\" with value=\"" + setting.second + "\"");
	}
}

esl::io::Input RequestHandler::accept(esl::com::http::server::RequestContext& requestContext) const {
	const auto& request = requestContext.getRequest();
	const std::map<std::string, std::string>& headers = request.getHeaders();
	std::size_t index = 0;

	logger.info << "LOCAL_PATH:     " << requestContext.getPath() << "\n";
	logger.info << "FULL_PATH:      " << request.getPath() << "\n";
	logger.info << "HTTP_VERSION:   " << request.getHTTPVersion() << "\n";
	logger.info << "METHOD:         " << request.getMethod().toString() << "\n";
	logger.info << "HOST_ADDRESS:   " << request.getHostAddress() << "\n";
	logger.info << "HOST_PORT:      " << request.getHostPort() << "\n";
	logger.info << "REMOTE_ADDRESS: " << request.getRemoteAddress() << "\n";
	logger.info << "REMOTE_PORT:    " << request.getRemotePort() << "\n";
	logger.info << "CONTENT_TYPE:   " << request.getContentType().toString() << "\n";
	logger.info << "HEADERS: (" << headers.size() << ")\n";
	for(const auto& header : headers) {
		logger.info << (++index) << ": " << header.first << "=" << header.second << "\n";
	}
	logger.info << std::flush;

	return esl::io::Input();
}

} /* namespace dump */
} /* namespace http */
} /* namespace builtin */
} /* namespace jerry */
