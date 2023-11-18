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

#include <openjerry/engine/http/RequestContext.h>
#include <openjerry/Logger.h>

#include <esl/io/Output.h>

namespace openjerry {
namespace engine {
namespace http {


namespace {
Logger logger("openjerry::engine::http::RequestContext");
} /* anonymous namespace */

RequestContext::RequestContext(esl::com::http::server::RequestContext& aRequestContext)
: baseRequestContext(aRequestContext),
  connection(*this, aRequestContext.getConnection()),
  path(aRequestContext.getPath())
{ }

esl::com::http::server::Connection& RequestContext::getConnection() const {
	return const_cast<Connection&>(connection);
}

const esl::com::http::server::Request& RequestContext::getRequest() const {
	return baseRequestContext.getRequest();
}

void RequestContext::setPath(std::string aPath) {
	path = std::move(aPath);
}

const std::string& RequestContext::getPath() const {
	return path;
}

esl::object::Context& RequestContext::getObjectContext() {
	return baseRequestContext.getObjectContext();
}

const esl::object::Context& RequestContext::getObjectContext() const {
	return baseRequestContext.getObjectContext();
}

void RequestContext::setHeadersContext(const Context* aHeadersContext) {
	headersContext = aHeadersContext;
}

const Context* RequestContext::getHeadersContext() const {
	return headersContext;
}

void RequestContext::setErrorHandlingContext(const Context* aErrorHandlingContext) {
	errorHandlingContext = aErrorHandlingContext;
}

const Context* RequestContext::getErrorHandlingContext() const {
	return errorHandlingContext;
}


} /* namespace http */
} /* namespace engine */
} /* namespace openjerry */
