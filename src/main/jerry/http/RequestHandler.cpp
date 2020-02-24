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

#include <jerry/http/RequestHandler.h>
#include <esl/Stacktrace.h>

namespace jerry {
namespace http {

Logger RequestHandler::logger("jerry::engine::RequestHandler");

RequestHandler::RequestHandler(std::unique_ptr<esl::http::server::RequestHandler> aRequestHandler, std::unique_ptr<esl::http::server::RequestContext> aRequestContext, const engine::Endpoint& aEngineEndpoint)
: esl::http::server::RequestHandler(),
  requestHandler(std::move(aRequestHandler)),
  requestContext(std::move(aRequestContext)),
  engineEndpoint(aEngineEndpoint)
{ }

bool RequestHandler::process(const char* contentData, std::size_t contentDataSize) {
	std::string exceptionMsg;
    std::unique_ptr<esl::Stacktrace> stacktrace = nullptr;

    try {
    	return requestHandler->process(contentData, contentDataSize);
    }
    catch (std::exception& e) {
    	exceptionMsg = e.what();

    	const esl::Stacktrace* stacktracePtr = esl::getStacktrace(e);
    	if(stacktracePtr) {
            stacktrace.reset(new esl::Stacktrace(*stacktracePtr));
    	}
    }
    catch (...) {
    	exceptionMsg = "unknown exception";
    }


	logger.error << exceptionMsg << std::endl;
    if(stacktrace) {
        logger.error << "Stacktrace:\n";
        stacktrace->dump(logger.error);
/*
		outputContent = NOT_FOUND_PAGE;

		std::unique_ptr<esl::http::server::ResponseStatic> response;
		response.reset(new esl::http::server::ResponseStatic(404, "text/html", NOT_FOUND_PAGE.data(), NOT_FOUND_PAGE.size()));
		connection.sendResponse(std::move(response));
*/
    }

	return false;
}

} /* namespace http */
} /* namespace jerry */
