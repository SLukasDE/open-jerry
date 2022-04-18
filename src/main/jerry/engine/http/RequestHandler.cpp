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

#include <jerry/engine/http/RequestHandler.h>
#include <jerry/engine/http/RequestContext.h>
#include <jerry/engine/http/Context.h>
#include <jerry/engine/http/InputProxy.h>
#include <jerry/engine/http/ExceptionHandler.h>
#include <jerry/Logger.h>

#include <esl/com/http/server/exception/StatusCode.h>

#include <string>
#include <memory>

namespace jerry {
namespace engine {
namespace http {


namespace {
Logger logger("jerry::engine::http::RequestHandler");
}

RequestHandler::RequestHandler(Context& aContext)
: context(aContext)
{ }

esl::io::Input RequestHandler::accept(esl::com::http::server::RequestContext& baseRequestContext) const {
	std::unique_ptr<RequestContext> requestContext(new RequestContext(baseRequestContext));

	try {
		/* Access log */
		logger.info << "Request for hostname " << baseRequestContext.getRequest().getHostName() << ": " << baseRequestContext.getRequest().getMethod() << " \"" << baseRequestContext.getRequest().getPath() << "\" received from " << baseRequestContext.getRequest().getRemoteAddress() << "\n";

		esl::io::Input input = context.accept(*requestContext);
		if(input) {
			return InputProxy::create(std::move(input), std::move(requestContext));
		}
		throw esl::com::http::server::exception::StatusCode(404);

	}
	catch(...) {
		ExceptionHandler exceptionHandler(std::current_exception());
    	exceptionHandler.dump(logger.error);
		exceptionHandler.dumpHttp(requestContext->getConnection(), requestContext->getErrorHandlingContext(), requestContext->getHeadersContext());
	}

	return esl::io::Input();
}


} /* namespace http */
} /* namespace engine */
} /* namespace jerry */
