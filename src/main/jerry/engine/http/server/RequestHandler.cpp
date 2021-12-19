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

#include <jerry/engine/http/server/RequestHandler.h>
#include <jerry/engine/http/server/RequestContext.h>
#include <jerry/engine/http/server/Socket.h>
#include <jerry/engine/http/server/Listener.h>
#include <jerry/engine/http/server/InputProxy.h>
#include <jerry/engine/http/server/ExceptionHandler.h>
#include <jerry/Logger.h>

#include <esl/com/http/server/exception/StatusCode.h>

#include <string>
#include <memory>

namespace jerry {
namespace engine {
namespace http {
namespace server {

namespace {
Logger logger("jerry::engine::http::server::RequestHandler");
}

RequestHandler::RequestHandler(Socket& aSocket)
: socket(aSocket)
{ }

esl::io::Input RequestHandler::accept(esl::com::http::server::RequestContext& baseRequestContext, esl::object::Interface::ObjectContext& objectContext) const {
	std::unique_ptr<RequestContext> requestContext(new RequestContext(baseRequestContext));

	try {
		/* Access log */
		logger.info << "Request for hostname " << baseRequestContext.getRequest().getHostName() << ": " << baseRequestContext.getRequest().getMethod() << " \"" << baseRequestContext.getRequest().getPath() << "\" received from " << baseRequestContext.getRequest().getRemoteAddress() << "\n";

		/* *************************************************** *
		 * Lookup corresponding Listener matching the hostname *
		 * *************************************************** */
		logger.debug << "Lookup http-listerner by host name \"" << baseRequestContext.getRequest().getHostName() << "\"\n";

		Listener* listener = socket.getListenerByHostname(baseRequestContext.getRequest().getHostName());
		if(listener == nullptr) {
			throw esl::com::http::server::exception::StatusCode(500, "No http-listener found for request to \"" + baseRequestContext.getRequest().getHostName() + ":" + std::to_string(baseRequestContext.getRequest().getHostPort()));
		}

		logger.debug << "HTTP listener found\n";

		esl::io::Input input = listener->accept(*requestContext);
		if(input) {
			return InputProxy::create(std::move(input), std::move(requestContext));
		}
	}
	catch(...) {
		ExceptionHandler exceptionHandler(std::current_exception());
    	exceptionHandler.engine::ExceptionHandler::dump(logger.error);
		exceptionHandler.dump(*requestContext);
	}

	return esl::io::Input();
}

} /* namespace server */
} /* namespace http */
} /* namespace engine */
} /* namespace jerry */
