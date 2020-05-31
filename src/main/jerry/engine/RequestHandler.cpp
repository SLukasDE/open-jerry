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

#include <jerry/engine/RequestHandler.h>
#include <jerry/engine/RequestContext.h>
#include <jerry/engine/Context.h>
#include <jerry/engine/Endpoint.h>
#include <jerry/engine/Listener.h>
#include <jerry/engine/ExceptionHandler.h>
#include <jerry/http/StatusCode.h>
#include <jerry/utility/URL.h>
#include <jerry/utility/MIME.h>
#include <jerry/Logger.h>

#include <esl/http/server/ResponseDynamic.h>
#include <esl/http/server/ResponseStatic.h>
#include <esl/http/server/exception/StatusCode.h>
#include <esl/utility/String.h>
#include <esl/database/exception/SqlError.h>

#include <sstream>

namespace jerry {
namespace engine {

namespace {

Logger logger("jerry::engine::RequestHandler");

const std::string PAGE_301(
		"<!DOCTYPE html>\n"
		"<html>\n"
		"<head>\n"
		"<title>301</title>\n"
		"</head>\n"
		"<body>\n"
		"<h1>301</h1>\n"
		"</body>\n"
		"</html>\n");

std::vector<std::string> makePathList(const std::string& path) {
	return esl::utility::String::split(esl::utility::String::trim(path, '/'), '/');
}

bool checkIfEndingWithSlash(const std::string& path) {
	if(path.empty()) {
		return false;
	}
	return path.at(path.size()-1) == '/';
}

std::unique_ptr<esl::Stacktrace> createStackstrace(const esl::Stacktrace* stacktracePtr) {
	if(stacktracePtr) {
		return std::unique_ptr<esl::Stacktrace>(new esl::Stacktrace(*stacktracePtr));
	}

	return nullptr;
}

} /* anonymous namespace */

RequestHandler::RequestHandler(const Listener& listener, esl::http::server::RequestContext& baseRequestContext)
: esl::http::server::requesthandler::Interface::RequestHandler(),
  // requestContext(std::unique_ptr<esl::http::server::RequestContext>(new RequestContext(baseRequestContext, listener))),
  requestContext(baseRequestContext, listener),
  engineEndpoint(listener),
  pathList(makePathList(baseRequestContext.getRequest().getPath())),
  isEndingWithSlash(checkIfEndingWithSlash(baseRequestContext.getRequest().getPath()))
{ }

void RequestHandler::setContext(const Context& context) {
	requestContext.setEngineContext(context);
}

void RequestHandler::setEndpoint(const Endpoint& endpoint) {
	engineEndpoint = std::cref(endpoint);

	std::size_t depth = engineEndpoint.get().getDepth() + engineEndpoint.get().getPathList().size();
	std::string path;

	if(depth < pathList.size()) {
		path += pathList[depth];
	}
	for(std::size_t i = depth+1; i < pathList.size(); ++i) {
		path += "/" + pathList[i];
	}
	if(isEndingWithSlash) {
		path += "/";
	}

	requestContext.setPath(path);
}

void RequestHandler::setRequestHandler(esl::http::server::requesthandler::Interface::CreateRequestHandler createRequestHandler) {
	ExceptionHandler exceptionHandler;

    /* setting 'exceptionOccured' is important to make hasRequestHandler() return true */
    exceptionOccured = exceptionHandler.call(
			[this, createRequestHandler]() { this->requestHandler = createRequestHandler(requestContext); },
			requestContext.getConnection());

    if(exceptionOccured) {
		/* **************** *
		 * Output on logger *
		 * **************** */
    	exceptionHandler.setShowException(true);
    	exceptionHandler.setShowStacktrace(true);

    	exceptionHandler.dump(logger.warn);

		/* ************* *
		* HTTP Response *
		* ************* */
    	exceptionHandler.setShowException(engineEndpoint.get().getShowException());
    	exceptionHandler.setShowStacktrace(engineEndpoint.get().getShowStacktrace());

		exceptionHandler.dump(
				requestContext.getConnection(),
				[this](unsigned short statusCode) { return engineEndpoint.get().findErrorDocument(statusCode); });
    }
}

bool RequestHandler::hasRequestHandler() const {
	return (requestHandler || exceptionOccured) ? true : false;
}

const std::vector<std::string>& RequestHandler::getPathList() {
	return pathList;
}

bool RequestHandler::process(const char* contentData, std::size_t contentDataSize) {
	ExceptionHandler exceptionHandler;

	bool rv = false;
	if(exceptionHandler.call(
			[this, &rv, contentData, contentDataSize]() {
				if(requestHandler) {
					rv = requestHandler->process(contentData, contentDataSize);
				}
			},
			requestContext.getConnection())
			) {
		/* **************** *
		 * Output on logger *
		 * **************** */
		exceptionHandler.setShowException(true);
		exceptionHandler.setShowStacktrace(true);

    	exceptionHandler.dump(logger.warn);

		/* ************* *
		* HTTP Response *
		* ************* */
    	exceptionHandler.setShowException(engineEndpoint.get().getShowException());
    	exceptionHandler.setShowStacktrace(engineEndpoint.get().getShowStacktrace());

		exceptionHandler.dump(
				requestContext.getConnection(),
				[this](unsigned short statusCode) { return engineEndpoint.get().findErrorDocument(statusCode); });
	}
	return rv;
}

} /* namespace engine */
} /* namespace jerry */
