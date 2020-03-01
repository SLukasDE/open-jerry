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

#include <jerry/engine/Endpoint.h>
#include <jerry/engine/Listener.h>
#include <jerry/engine/Engine.h>
#include <jerry/Logger.h>
#include <esl/utility/String.h>
//#include <stdexcept>

namespace jerry {
namespace engine {

namespace {
Logger logger("jerry::engine::Endpoint");
} /* anonymous namespace */

Endpoint::Endpoint(Listener& listener, Context* parent, std::string aPath)
: Context(listener, parent),
  pathList(esl::utility::String::split(esl::utility::String::trim(std::move(aPath), '/'), '/'))
{
	if(parent) {
		listener.registerEndpoint(*this, getEndpointPathList());
	}
}

std::unique_ptr<esl::http::server::RequestHandler> Endpoint::createRequestHandler(esl::http::server::RequestContext& requestContext, const std::string& path) const {
	logger.trace << "path=\"" << path << "\"\n";
	logger.trace << "within pathList=\"";
	for(const auto& pathEntry : pathList) {
		logger.trace << pathEntry << "/";
	}
	logger.trace << "\"\n";

	std::unique_ptr<esl::http::server::RequestHandler> requestHandler = Context::createRequestHandler(requestContext, path, *this);

	// TODO: special endpoint stuff, like catching exceptions, returning a default error page, show stacktrace, ...

	if(!requestHandler) {
		logger.trace << "requestHandler == nullptr\n";
		// ...
	}

	return requestHandler;
}

std::vector<std::string> Endpoint::getEndpointPathList() const {
	std::vector<std::string> result = Context::getEndpointPathList();
	result.insert(std::end(result), std::begin(pathList), std::end(pathList));

	return result;
}

std::unique_ptr<esl::http::server::RequestHandler> Endpoint::createRequestHandler(esl::http::server::RequestContext& requestContext, const std::string& path, const Endpoint& endpoint) const {
	return nullptr;
}

} /* namespace engine */
} /* namespace jerry */
