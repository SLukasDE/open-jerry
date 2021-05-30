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

#include <jerry/engine/http/server/Writer.h>
#include <jerry/engine/http/server/Endpoint.h>
#include <jerry/engine/http/server/Listener.h>
#include <jerry/engine/http/server/ExceptionHandler.h>
#include <jerry/Logger.h>

namespace jerry {
namespace engine {
namespace http {
namespace server {

namespace {
Logger logger("jerry::engine::http::server::Writer");
} /* anonymous namespace */

Writer::Writer(const Listener& listener, esl::http::server::RequestContext& aRequestContext)
: requestContext(aRequestContext, *this, listener)
{ }

RequestContext& Writer::getRequestContext() {
	return requestContext;
}

std::size_t Writer::write(const void* contentData, std::size_t contentDataSize) {
	std::size_t rv = esl::io::Writer::npos;

	ExceptionHandler exceptionHandler;
	if(exceptionHandler.call(
			[this, &rv, contentData, contentDataSize]() {
				if(requestContext.getInput()) {
					rv = requestContext.getInput().getWriter().write(contentData, contentDataSize);
				}
			}
			)) {
		/* **************** *
		 * Output on logger *
		 * **************** */
		exceptionHandler.setShowException(true);
		exceptionHandler.setShowStacktrace(true);

		// TODO
		// This cast is a workaround to avoid a compile time error.
		// Why does the compiler not find the matching dump-method in the base class?
    	//exceptionHandler.dump(logger.warn);
    	engine::ExceptionHandler& e(exceptionHandler);
    	e.dump(logger.warn);

		/* ************* *
		* HTTP Response *
		* ************* */
    	exceptionHandler.setShowException(requestContext.getEndpoint().getShowException());
    	exceptionHandler.setShowStacktrace(requestContext.getEndpoint().getShowStacktrace());

		exceptionHandler.dump(
				requestContext.getConnection(),
				[this](unsigned short statusCode) { return requestContext.getEndpoint().findErrorDocument(statusCode); });
	}
	return rv;
}

std::size_t Writer::getSizeWritable() const {
	if(requestContext.getInput()) {
		return requestContext.getInput().getWriter().getSizeWritable();
	}
	return esl::io::Writer::npos;
}

} /* namespace server */
} /* namespace http */
} /* namespace engine */
} /* namespace jerry */
