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
#include <jerry/Logger.h>
#include <esl/Stacktrace.h>

namespace jerry {
namespace http {

namespace {
Logger logger("jerry::engine::RequestHandler");
} /* anonymous namespace */

RequestHandler::RequestHandler(std::unique_ptr<esl::http::server::RequestHandler> aRequestHandler, std::unique_ptr<esl::http::server::RequestContext> aRequestContext, const engine::Endpoint& aEngineEndpoint)
: esl::http::server::RequestHandler(),
  requestHandler(std::move(aRequestHandler)),
  requestContext(std::move(aRequestContext)),
  engineEndpoint(aEngineEndpoint)
{ }

bool RequestHandler::process(const char* contentData, std::size_t contentDataSize) {
	std::string exceptionMsg;
    std::unique_ptr<esl::Stacktrace> stacktrace = nullptr;
    logger.trace << "process begin\n";

    try {
        logger.trace << "process requestHandler->process\n";
    	return requestHandler->process(contentData, contentDataSize);
    }
    catch (std::exception& e) {
        logger.trace << "process std::exception\n";
    	exceptionMsg = e.what();

    	const esl::Stacktrace* stacktracePtr = esl::getStacktrace(e);
    	if(stacktracePtr) {
            stacktrace.reset(new esl::Stacktrace(*stacktracePtr));
    	}
    }
    catch (...) {
        logger.trace << "process unknown exception\n";
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

    logger.trace << "process end\n";
	return false;
}

} /* namespace http */
} /* namespace jerry */
