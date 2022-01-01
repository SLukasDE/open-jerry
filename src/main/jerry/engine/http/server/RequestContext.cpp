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

#include <jerry/engine/http/server/RequestContext.h>
#include <jerry/Logger.h>

namespace jerry {
namespace engine {
namespace http {
namespace server {

namespace {
Logger logger("jerry::engine::http::server::RequestContext");
} /* anonymous namespace */

RequestContext::RequestContext(esl::com::http::server::RequestContext& aRequestContext)
: requestContext(aRequestContext),
  connection(requestContext.getConnection(), headersAndErrorHandlingContext),
  path(requestContext.getPath())
{ }

esl::com::http::server::Connection& RequestContext::getConnection() const {
	return const_cast<jerry::engine::http::server::Connection&>(connection);
}

const esl::com::http::server::Request& RequestContext::getRequest() const {
	return requestContext.getRequest();
}

void RequestContext::setPath(std::string aPath) {
	path = std::move(aPath);
}

const std::string& RequestContext::getPath() const {
	return path;
}

esl::object::Interface::ObjectContext& RequestContext::getObjectContext() {
	return objectContext;
}

const Context& RequestContext::getHeadersAndErrorHandlingContext() const {
	return headersAndErrorHandlingContext;
}

void RequestContext::setParentHeadersAndErrorHandlingContext(Context* aHeadersAndErrorHandlingContext) {
	headersAndErrorHandlingContext.setParent(aHeadersAndErrorHandlingContext);
}

} /* namespace server */
} /* namespace http */
} /* namespace engine */
} /* namespace jerry */
