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

#include <jerry/engine/basic/server/RequestHandler.h>
#include <jerry/engine/basic/server/RequestContext.h>
#include <jerry/engine/basic/server/Socket.h>
#include <jerry/engine/basic/server/InputProxy.h>
#include <jerry/engine/ExceptionHandler.h>
#include <jerry/Logger.h>

#include <string>
#include <memory>
#include <stdexcept>

namespace jerry {
namespace engine {
namespace basic {
namespace server {

namespace {
Logger logger("jerry::engine::basic::server::RequestHandler");
}

RequestHandler::RequestHandler(Socket& aSocket)
: socket(aSocket)
{ }

esl::io::Input RequestHandler::accept(esl::com::basic::server::RequestContext& baseRequestContext, esl::object::Interface::ObjectContext& objectContext) const {
	std::unique_ptr<RequestContext> requestContext(new RequestContext(baseRequestContext));

	try {
		/* Access log */
		logger.debug << "Request received\n";
#if 0
		const Listener* listener = socket.getListener();
		if(listener == nullptr) {
			throw std::runtime_error("No basic-listener found for incoming request");
		}

		esl::io::Input input = listener->accept(*requestContext);
#endif
		esl::io::Input input = socket.getContext().accept(*requestContext);

		if(input) {
			return InputProxy::create(std::move(input), std::move(requestContext));
		}
	}
	catch(...) {
		ExceptionHandler exceptionHandler(std::current_exception());
    	exceptionHandler.engine::ExceptionHandler::dump(logger.error);
		//exceptionHandler.dump(*requestContext);
	}

	return esl::io::Input();
}

std::set<std::string> RequestHandler::getNotifiers() const {
	return socket.getNotifiers();
}

} /* namespace server */
} /* namespace basic */
} /* namespace engine */
} /* namespace jerry */
