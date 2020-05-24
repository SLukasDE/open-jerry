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
#include <jerry/html/HTML.h>
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
	callWithExceptionHandler([this, createRequestHandler] {
		requestHandler = createRequestHandler(requestContext);
		return true;
	});
}

bool RequestHandler::hasRequestHandler() const {
	return (requestHandler || exceptionOccured) ? true : false;
}

const std::vector<std::string>& RequestHandler::getPathList() {
	return pathList;
}

bool RequestHandler::process(const char* contentData, std::size_t contentDataSize) {
	return callWithExceptionHandler([this, contentData, contentDataSize] {
		if(requestHandler) {
			return requestHandler->process(contentData, contentDataSize);
		}
		return false;
	});
}
#if 0
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
#endif

bool RequestHandler::callWithExceptionHandler(std::function<bool()> callFunction) {
	bool rc = false;

	unsigned short statusCode = 500;
	esl::utility::MIME contentType;

	std::string title;
	std::string exceptionMsg;
	std::string exceptionDetails;

	std::unique_ptr<esl::Stacktrace> stacktrace;

    try {
    	return callFunction();
    }
	catch(const esl::http::server::exception::StatusCode& e) {
		rc = true;
		statusCode = e.getStatusCode();
		contentType = e.getMimeType();
#if 0
		title = std::to_string(e.getStatusCode()) + " ";
		if(e.what() && std::string(e.what()) != esl::http::server::exception::StatusCode::getMessage(e.getStatusCode())) {
			title += e.what();
		}
		else {
			title += http::StatusCode::getMessage(e.getStatusCode());
		}

		exceptionMsg = title;
#endif
		title = std::to_string(e.getStatusCode()) + " " + http::StatusCode::getMessage(e.getStatusCode());

		if(e.what() && std::string(e.what()) != esl::http::server::exception::StatusCode::getMessage(e.getStatusCode())) {
			exceptionMsg = e.what();
		}
		else {
			exceptionMsg = http::StatusCode::getMessage(e.getStatusCode());
		}

		exceptionDetails.clear();
	}
	catch(const esl::database::exception::SqlError& e) {
		rc = false;
		statusCode = 500;
		contentType = esl::utility::MIME(esl::utility::MIME::textHtml);

		title = "SQL Error";

		exceptionMsg = e.what();

		std::stringstream s;
		s << "SQL Return Code:" << e.getSqlReturnCode() << "\n";
		const esl::database::Diagnostics& diagnostics = e.getDiagnostics();
		diagnostics.dump(s);
		exceptionDetails = s.str();

    	stacktrace = createStackstrace(esl::getStacktrace(e));
	}
    catch (std::exception& e) {
		rc = false;
		statusCode = 500;
		contentType = esl::utility::MIME(esl::utility::MIME::textHtml);

		title = "Exception Error";

    	exceptionMsg = e.what();

		exceptionDetails.clear();

    	stacktrace = createStackstrace(esl::getStacktrace(e));
    }
    catch (...) {
		rc = false;
		statusCode = 500;
		contentType = esl::utility::MIME(esl::utility::MIME::textHtml);

		title = "Unknown Exception Error";

    	exceptionMsg = "unknown exception";

		exceptionDetails.clear();
    }

    /* important to make hasRequestHandler() return true */
    exceptionOccured = true;

    /* **************** *
     * Output on logger *
     * **************** */
	logger.warn << title << std::endl;
	logger.warn << exceptionMsg << std::endl;
	if(!exceptionDetails.empty()) {
		logger.warn << "\n\n" << exceptionDetails << std::endl;
	}

    if(stacktrace) {
        logger.warn << "Stacktrace:\n";
        stacktrace->dump(logger.warn);
    }
    else {
        logger.warn << "Stacktrace: not available\n";
    }

    /* ************* *
     * HTTP Response *
     * ************* */

    const std::pair<std::string, bool>* errorDocument = engineEndpoint.get().findErrorDocument(statusCode);
    if(errorDocument) {
    	utility::URL url(errorDocument->first);

		if(url.getScheme() == "http" || url.getScheme() == "https") {
    		std::unique_ptr<esl::http::server::ResponseStatic> response;
    		response.reset(new esl::http::server::ResponseStatic(301, esl::utility::MIME::textHtml, PAGE_301.data(), PAGE_301.size()));
    		response->addHeader("Location", errorDocument->first);
    		requestContext.getConnection().sendResponse(std::move(response));

    		return rc;
    	}
    	if(url.getScheme().empty() || url.getScheme() == "file") {
    		/* if we don't need to parse the file, then we are done very quick */
    		if(errorDocument->second == false) {
            	esl::utility::MIME mime = utility::MIME::byFilename(url.getPath());
        		std::unique_ptr<esl::http::server::ResponseFile> response(new esl::http::server::ResponseFile(statusCode, mime.toString(), url.getPath()));
        		requestContext.getConnection().sendResponse(std::move(response));

        		return rc;
    		}

    		// TODO: URGENT, because errorDocument is still "!= nullptr" !
    		/* 1. we have to load the content of the file to a variable
    		 * 2. substitute the content of the variable
    		 * 3. store the result into 'outputContent'
    		 */
    	}
    	else {
    		/* switch to default error status page if there is no document defined for this status code */
    		errorDocument = nullptr;
    	}
    }

    std::string outputContent;
    if(errorDocument == nullptr) {
    	if(contentType == esl::utility::MIME(esl::utility::MIME::textHtml)) {
        	outputContent = "<!DOCTYPE html>\n"
        			"<html>\n"
        			"<head>\n";

        	outputContent += "<title>" + html::toHTML(title) + "</title>\n";
        	outputContent += "<body bgcolor=\"white\">\n";

        	/* print excpetion message, if enabled */
            if(engineEndpoint.get().getShowException()) {
            	outputContent += "<center><h1>" + html::toHTML(exceptionMsg) + "</h1></center>\n";

                if(!exceptionDetails.empty()) {
                	outputContent += "<hr>\n";
                	outputContent += html::toHTML(exceptionDetails) + "<br>\n";
                }
            }

            if(engineEndpoint.get().getShowStacktrace()) {
        		/* print stacktrace, if enabled */
        		if(stacktrace) {
        			outputContent += "Stacktrace:\n"
        					"<br>\n";

        			std::stringstream sstream;
        			stacktrace->dump(sstream);
        			outputContent += html::toHTML(sstream.str());
        		}
        		else {
        			outputContent += "Stacktrace: not available\n"
        					"<br>\n";
        		}
            }

        	outputContent += "<hr><center>jerry/0.1.0</center>\n";
        	outputContent += "</body>\n"
        			"</html>\n";
    	}
    	else {
        	outputContent = std::move(exceptionMsg);
    	}
    }



	std::unique_ptr<esl::http::server::ResponseDynamic> response;
	response.reset(new esl::http::server::ResponseDynamic(statusCode, contentType, std::move(outputContent)));
	requestContext.getConnection().sendResponse(std::move(response));

	//auto lambda = [this](char* buffer, std::size_t count) { return getData(buffer, count); };
	//response.reset(new esl::http::server::ResponseDynamic(statusCode, esl::utility::MIME::textHtml, lambda ));
	//requestContext.getConnection().sendResponse(std::move(response));

	return rc;
}

} /* namespace engine */
} /* namespace jerry */
