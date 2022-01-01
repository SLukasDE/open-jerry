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

#include <jerry/engine/http/server/ExceptionHandler.h>
#include <jerry/html/HTML.h>
#include <jerry/http/StatusCode.h>
#include <jerry/utility/MIME.h>
#include <jerry/Logger.h>

#include <esl/com/http/server/Response.h>
#include <esl/io/output/Memory.h>
#include <esl/io/output/String.h>
#include <esl/utility/URL.h>
#include <esl/utility/Protocol.h>
#include <esl/utility/MIME.h>
#include <esl/Stacktrace.h>

#include <sstream>

namespace jerry {
namespace engine {
namespace http {
namespace server {

namespace {
Logger logger("jerry::engine::http::server::ExceptionHandler");

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
} /* anonymous namespace */

ExceptionHandler::ExceptionHandler(std::exception_ptr exceptionPointer)
: engine::ExceptionHandler(exceptionPointer)
{ }

void ExceptionHandler::dump(const esl::com::http::server::RequestContext& requestContext, const Context& errorHandlingContext) const {
	initialize();

	const http::server::Document* errorDocument = errorHandlingContext.findErrorDocument(httpStatusCode);

	if(errorDocument) {
		esl::utility::URL url(errorDocument->getPath());

		if(url.getScheme() == esl::utility::Protocol::http || url.getScheme() == esl::utility::Protocol::https) {
			esl::com::http::server::Response response(301, esl::utility::MIME::textHtml);
			response.addHeader("Location", errorDocument->getPath());
			requestContext.getConnection().send(response, std::unique_ptr<esl::io::Producer>(new esl::io::output::Memory(PAGE_301.data(), PAGE_301.size())));

			return;
		}
		if(!url.getScheme() || url.getScheme() == esl::utility::Protocol::file) {
			/* if we don't need to parse the file, then we are done very quick */
			if(errorDocument->getLanguage().empty()) {
				esl::com::http::server::Response response(httpStatusCode, utility::MIME::byFilename(url.getPath()));
				requestContext.getConnection().send(response, url.getPath());

				return;
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

    std::string content;
	if(httpContentType == esl::utility::MIME::textHtml) {
	    content = getHTMLContent(errorHandlingContext.getShowException(), errorHandlingContext.getShowStacktrace());
	}
	else if(httpContentType == esl::utility::MIME::textPlain) {
	    content = getTextContent(errorHandlingContext.getShowException(), errorHandlingContext.getShowStacktrace());
	}
	else {
		content = httpMessage;
	}

	esl::com::http::server::Response response(httpStatusCode, httpContentType);
	requestContext.getConnection().send(response, std::unique_ptr<esl::io::Producer>(new esl::io::output::String(std::move(content))));
}

void ExceptionHandler::initializeMessage() const {
	engine::ExceptionHandler::initializeMessage();

	httpStatusCode = 500;
	httpContentType = esl::utility::MIME(esl::utility::MIME::textHtml);
	httpTitle = "Unknown Exception Error";
	httpMessage = "unknown exception";
}

void ExceptionHandler::initializeMessage(const esl::com::http::server::exception::StatusCode& e) const {
	engine::ExceptionHandler::initializeMessage(e);

	httpStatusCode = e.getStatusCode();
	httpContentType = e.getMimeType();
	httpTitle = std::to_string(e.getStatusCode()) + " " + jerry::http::StatusCode::getMessage(e.getStatusCode());
	if(e.what() && std::string(e.what()) != esl::com::http::server::exception::StatusCode::getMessage(e.getStatusCode())) {
		httpMessage = e.what();
	}
	else {
		httpMessage = jerry::http::StatusCode::getMessage(e.getStatusCode());
	}
}

void ExceptionHandler::initializeMessage(const esl::database::exception::SqlError& e) const {
	engine::ExceptionHandler::initializeMessage(e);

	httpStatusCode = 500;
	httpContentType = esl::utility::MIME(esl::utility::MIME::textHtml);
	httpTitle = "SQL Error";
	httpMessage = e.what();
}

void ExceptionHandler::initializeMessage(const std::runtime_error& e) const {
	engine::ExceptionHandler::initializeMessage(e);

	httpStatusCode = 500;
	httpContentType = esl::utility::MIME(esl::utility::MIME::textHtml);
	httpTitle = "Runtime error";
	httpMessage = e.what();
}

void ExceptionHandler::initializeMessage(const std::exception& e) const {
	engine::ExceptionHandler::initializeMessage(e);

	httpStatusCode = 500;
	httpContentType = esl::utility::MIME(esl::utility::MIME::textHtml);
	httpTitle = "Exception";
	httpMessage = e.what();
}

std::string ExceptionHandler::getHTMLContent(bool showException, bool showStacktrace) const {
	std::string outputContent;

	outputContent = "<!DOCTYPE html>\n"
			"<html>\n"
			"<head>\n";

	outputContent += "<title>" + html::toHTML(httpTitle) + "</title>\n";
	outputContent += "<body bgcolor=\"white\">\n";

	/* print excpetion message, if enabled */
	if(showException) {
		outputContent += "<center><h1>\n";
		outputContent += html::toHTML(httpMessage) + "\n";
		outputContent += "</h1></center>\n";

		if(!getDetails().empty()) {
			outputContent += "<hr>\n";
			outputContent += html::toHTML(getDetails()) + "<br>\n";
		}
	}

	/* print stacktrace, if enabled */
	if(showStacktrace) {
		if(getStacktrace().empty()) {
			outputContent += "Stacktrace: not available\n"
					"<br>\n";
		}
		else {
			outputContent += "Stacktrace:\n"
					"<br>\n";
			outputContent += html::toHTML(getStacktrace());
		}
	}

	outputContent += "<hr><center>jerry/0.1.0</center>\n";
	outputContent += "</body>\n"
			"</html>\n";

    return outputContent;
}

std::string ExceptionHandler::getTextContent(bool showException, bool showStacktrace) const {
	std::string content;

	content = "jerry/0.1.0: " + httpTitle + "\n";
	content += "Status code: " + std::to_string(httpStatusCode) + "\n";

	/* print excpetion message, if enabled */
	if(showException) {
		content += "\n\n";
		content += "Exception: " + httpMessage;

		if(!getDetails().empty()) {
			content += "\n\nDetails:\n";
			content += getDetails() + "\n";
		}
	}

	/* print stacktrace, if enabled */
	if(showStacktrace) {
		content += "\n\n";
		if(getStacktrace().empty()) {
			content += "Stacktrace: not available\n";
		}
		else {
			content += "Stacktrace:\n";
			content += getStacktrace();
		}
	}

	return content;
}

} /* namespace server */
} /* namespace http */
} /* namespace engine */
} /* namespace jerry */
