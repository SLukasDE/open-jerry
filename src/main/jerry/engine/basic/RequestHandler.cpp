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

#include <jerry/engine/basic/RequestHandler.h>
#include <jerry/engine/basic/Socket.h>
#include <jerry/engine/InputProxy.h>
#include <jerry/engine/ExceptionHandler.h>
#include <jerry/Logger.h>

#include <memory>
#include <stdexcept>

namespace jerry {
namespace engine {
namespace basic {


namespace {
Logger logger("jerry::engine::basic::RequestHandler");
}

RequestHandler::RequestHandler(Socket& aSocket)
: socket(aSocket)
{ }

esl::io::Input RequestHandler::accept(esl::com::basic::server::RequestContext& requestContext) const {
	try {
		/* Access log */
		logger.debug << "Request received\n";

		esl::io::Input input = socket.getContext().accept(requestContext);
		if(input) {
			return InputProxy::create(std::move(input));
		}
	}
	catch(...) {
		ExceptionHandler exceptionHandler(std::current_exception());
    	exceptionHandler.engine::ExceptionHandler::dump(logger.error);
	}

	return esl::io::Input();
}

std::set<std::string> RequestHandler::getNotifiers() const {
	return socket.getNotifiers();
}


} /* namespace basic */
} /* namespace engine */
} /* namespace jerry */
