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
#include <jerry/Logger.h>
#include <esl/http/server/ResponseDynamic.h>
#include <esl/http/server/ResponseStatic.h>
#include <esl/utility/String.h>
#include <sstream>

namespace jerry {
namespace engine {

namespace {
Logger logger("jerry::engine::RequestHandler");

const std::string PAGE_500(
		"<!DOCTYPE html>\n"
		"<html>\n"
		"<head>\n"
		"<title>500</title>\n"
		"</head>\n"
		"<body>\n"
		"<h1>500</h1>\n"
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
	std::string exceptionMsg;
    std::unique_ptr<esl::Stacktrace> stacktrace = nullptr;

    try {
    	requestHandler = createRequestHandler(requestContext);
    	return;
    }
    catch (std::exception& e) {
    	exceptionMsg = e.what();

    	const esl::Stacktrace* stacktracePtr = esl::getStacktrace(e);
    	if(stacktracePtr) {
            stacktrace.reset(new esl::Stacktrace(*stacktracePtr));
    	}
    }
    catch (...) {
    	exceptionMsg = "unknown exception";
    }

    sendExceptionPage(std::move(exceptionMsg), std::move(stacktrace));
}

bool RequestHandler::hasRequestHandler() const {
	return requestHandler ? true : false;
}

const std::vector<std::string>& RequestHandler::getPathList() {
	return pathList;
}

bool RequestHandler::process(const char* contentData, std::size_t contentDataSize) {
	std::string exceptionMsg;
    std::unique_ptr<esl::Stacktrace> stacktrace = nullptr;

    try {
    	return requestHandler->process(contentData, contentDataSize);
    }
    catch (std::exception& e) {
    	exceptionMsg = e.what();

    	const esl::Stacktrace* stacktracePtr = esl::getStacktrace(e);
    	if(stacktracePtr) {
            stacktrace.reset(new esl::Stacktrace(*stacktracePtr));
    	}
    }
    catch (...) {
    	exceptionMsg = "unknown exception";
    }

    sendExceptionPage(std::move(exceptionMsg), std::move(stacktrace));

	return false;
}

int RequestHandler::getData(char* buffer, std::size_t bufferSize) {
    std::size_t remainingSize = outputContent.size() - outputPos;

    if(bufferSize > remainingSize) {
        bufferSize = remainingSize;
    }
    std::memcpy(buffer, &outputContent.data()[outputPos], bufferSize);
    outputPos += bufferSize;

    /* Hier wissen wir, dass keine Daten mehr zu senden sind, wenn bufferSize == 0 ist */
    /* Daher geben wir dann auch -1 zurueck - was dem Aufrufer signalisiert, dass keine Daten mehr zu senden sind */
    if(bufferSize == 0) {
        return -1;
    }
    return bufferSize;
}

void RequestHandler::sendExceptionPage(std::string exceptionMsg, std::unique_ptr<esl::Stacktrace> stacktrace) {
	logger.warn << exceptionMsg << std::endl;
    if(stacktrace) {
        logger.warn << "Stacktrace:\n";
        stacktrace->dump(logger.warn);
    }
    else {
        logger.warn << "Stacktrace: not available\n";
    }

    if(engineEndpoint.get().getShowException()) {
		outputContent.clear();
		outputContent += "<!DOCTYPE html>\n"
				"<html>\n"
				"<head>\n"
				"<title>500</title>\n"
				"</head>\n"
				"<body>\n";

		/* print excpetion message, if enabled */
		outputContent += "Exception: " + exceptionMsg + "\n"
				"<br>\n";

		/* print stacktrace, if enabled */
		if(stacktrace) {
			outputContent += "Stacktrace:\n"
					"<br>\n";
			std::stringstream sstream;
			stacktrace->dump(sstream);
			outputContent += sstream.str();
		}
		else {
			outputContent += "Stacktrace: not available\n"
					"<br>\n";
		}

		outputContent += "</body>\n"
				"</html>\n";

		std::unique_ptr<esl::http::server::ResponseDynamic> response;
		auto lambda = [this](char* buffer, std::size_t count) { return getData(buffer, count); };
		response.reset(new esl::http::server::ResponseDynamic(500, "text/html", lambda ));
		requestContext.getConnection().sendResponse(std::move(response));
    }
    else {
    	std::unique_ptr<esl::http::server::ResponseStatic> response;
    	response.reset(new esl::http::server::ResponseStatic(500, "text/html", PAGE_500.data(), PAGE_500.size()));
    	requestContext.getConnection().sendResponse(std::move(response));
    }
}

} /* namespace engine */
} /* namespace jerry */
