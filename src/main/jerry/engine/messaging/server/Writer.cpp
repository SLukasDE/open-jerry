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

#include <jerry/engine/messaging/server/Writer.h>
#include <jerry/engine/messaging/server/Listener.h>
#include <jerry/engine/ExceptionHandler.h>
#include <jerry/Logger.h>

namespace jerry {
namespace engine {
namespace messaging {
namespace server {

namespace {
Logger logger("jerry::engine::messaging::server::Writer");
} /* anonymous namespace */

Writer::Writer(const Listener& listener, esl::messaging::server::RequestContext& aRequestContext)
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

    	exceptionHandler.dump(logger.warn);
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
} /* namespace messaging */
} /* namespace engine */
} /* namespace jerry */
