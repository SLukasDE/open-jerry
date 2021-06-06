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

#include <jerry/engine/http/server/ExceptionHandler.h>
#include <jerry/html/HTML.h>
#include <jerry/http/StatusCode.h>
#include <jerry/utility/URL.h>
#include <jerry/utility/MIME.h>
#include <jerry/Logger.h>

#include <esl/com/http/server/Response.h>
#include <esl/com/http/server/exception/StatusCode.h>
#include <esl/com/http/server/exception/Interface.h>
#include <esl/database/exception/SqlError.h>
#include <esl/io/Producer.h>
#include <esl/io/output/Memory.h>
#include <esl/io/output/String.h>
#include <esl/utility/MIME.h>
#include <esl/Stacktrace.h>
#include <sstream>
#include <vector>
#include <functional>

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

std::unique_ptr<esl::Stacktrace> createStackstrace(const esl::Stacktrace* stacktracePtr) {
	if(stacktracePtr) {
		return std::unique_ptr<esl::Stacktrace>(new esl::Stacktrace(*stacktracePtr));
	}
	return nullptr;
}

const std::vector<std::reference_wrapper<const esl::com::http::server::exception::Interface>>& getExceptionMessageInterfaces() {
	static std::unique_ptr<std::vector<std::reference_wrapper<const esl::com::http::server::exception::Interface>>> exceptionMessageInterfaces;

	if(!exceptionMessageInterfaces) {
		exceptionMessageInterfaces.reset(new std::vector<std::reference_wrapper<const esl::com::http::server::exception::Interface>>);

		for(const auto& metaInterface : esl::getModule().getMetaInterfaces()) {
			if(metaInterface.type != esl::com::http::server::exception::Interface::getType()) {
				continue;
			}

			const esl::com::http::server::exception::Interface& exceptionMessageInterface = esl::getModule().getInterface<esl::com::http::server::exception::Interface>(metaInterface.implementation);
			exceptionMessageInterfaces->emplace_back(exceptionMessageInterface);
		}
	}

	return *exceptionMessageInterfaces;
}

} /* anonymous namespace */

void ExceptionHandler::dump(esl::com::http::server::Connection& connection, std::function<const http::server::Document*(unsigned short statusCode)> findDocument) const {
	const http::server::Document* errorDocument = nullptr;
	if(findDocument) {
		errorDocument = findDocument(httpMessage.statusCode);
	}

	if(errorDocument) {
		utility::URL url(errorDocument->getPath());

		if(url.getScheme() == "http" || url.getScheme() == "https") {
			esl::com::http::server::Response response(301, esl::utility::MIME::textHtml);
			response.addHeader("Location", errorDocument->getPath());
			connection.send(response, std::unique_ptr<esl::io::Producer>(new esl::io::output::Memory(PAGE_301.data(), PAGE_301.size())));

			return;
		}
		if(url.getScheme().empty() || url.getScheme() == "file") {
			/* if we don't need to parse the file, then we are done very quick */
			if(errorDocument->getLanguage().empty()) {
				esl::com::http::server::Response response(httpMessage.statusCode, utility::MIME::byFilename(url.getPath()));
				connection.send(response, url.getPath());

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
	if(httpMessage.contentType == esl::utility::MIME::textHtml) {
	    content = getHTMLContent();
	}
	else if(httpMessage.contentType == esl::utility::MIME::textPlain) {
	    content = getTextContent();
	}
	else {
		content = httpMessage.message;
	}

	esl::com::http::server::Response response(httpMessage.statusCode, httpMessage.contentType);
	connection.send(response, std::unique_ptr<esl::io::Producer>(new esl::io::output::String(std::move(content))));
}

void ExceptionHandler::setMessage() {
	engine::ExceptionHandler::setMessage();

	httpMessage.statusCode = 500;
	httpMessage.contentType = esl::utility::MIME(esl::utility::MIME::textHtml);
	httpMessage.title = "Unknown Exception Error";
	httpMessage.message = "unknown exception";
	httpMessage.details.clear();
	httpMessage.stacktrace.reset();
}

void ExceptionHandler::setMessage(const esl::com::http::server::exception::StatusCode& e) {
	engine::ExceptionHandler::setMessage(e);

	httpMessage.statusCode = e.getStatusCode();
	httpMessage.contentType = e.getMimeType();
	httpMessage.title = std::to_string(e.getStatusCode()) + " " + jerry::http::StatusCode::getMessage(e.getStatusCode());
	if(e.what() && std::string(e.what()) != esl::com::http::server::exception::StatusCode::getMessage(e.getStatusCode())) {
		httpMessage.message = e.what();
	}
	else {
		httpMessage.message = jerry::http::StatusCode::getMessage(e.getStatusCode());
	}
	httpMessage.details.clear();
	httpMessage.stacktrace = createStackstrace(esl::getStacktrace(e));
}

void ExceptionHandler::setMessage(const esl::database::exception::SqlError& e) {
	engine::ExceptionHandler::setMessage(e);

	httpMessage.statusCode = 500;
	httpMessage.contentType = esl::utility::MIME(esl::utility::MIME::textHtml);
	httpMessage.title = "SQL Error";
	httpMessage.message = e.what();
	httpMessage.details = getMessage().message;
	httpMessage.stacktrace = createStackstrace(esl::getStacktrace(e));
}

void ExceptionHandler::setMessage(const std::runtime_error& e) {
	engine::ExceptionHandler::setMessage(e);

	httpMessage.statusCode = 500;
	httpMessage.contentType = esl::utility::MIME(esl::utility::MIME::textHtml);
	httpMessage.title = "std::runtime_error";
	httpMessage.message = e.what();
	httpMessage.details.clear();
	httpMessage.stacktrace = createStackstrace(esl::getStacktrace(e));
}

void ExceptionHandler::setMessage(const std::exception& e) {
	engine::ExceptionHandler::setMessage(e);

	bool httpMessageFound = false;
	for(auto exceptionMessageInterface : getExceptionMessageInterfaces()) {
		//std::unique_ptr<esl::http::server::exception::Interface::Message> messagePtr;
		auto messagePtr = exceptionMessageInterface.get().createMessage(e);
		if(messagePtr) {
			httpMessage = std::move(*messagePtr);
			httpMessageFound = true;
			break;
		}
	}
	if(httpMessageFound == false) {
		httpMessage.statusCode = 500;
		httpMessage.contentType = esl::utility::MIME(esl::utility::MIME::textHtml);
		httpMessage.title = "std::exception";
		httpMessage.message = e.what();
		httpMessage.details.clear();
		httpMessage.stacktrace = createStackstrace(esl::getStacktrace(e));
	}
}

std::string ExceptionHandler::getHTMLContent() const {
	std::string outputContent;

	outputContent = "<!DOCTYPE html>\n"
			"<html>\n"
			"<head>\n";

	outputContent += "<title>" + html::toHTML(httpMessage.title) + "</title>\n";
	outputContent += "<body bgcolor=\"white\">\n";

	/* print excpetion message, if enabled */
	if(getShowException()) {
		outputContent += "<center><h1>\n";
		outputContent += html::toHTML(httpMessage.message) + "\n";
		outputContent += "</h1></center>\n";

		if(!httpMessage.details.empty()) {
			outputContent += "<hr>\n";
			outputContent += html::toHTML(httpMessage.details) + "<br>\n";
		}
	}

	/* print stacktrace, if enabled */
	if(getShowStacktrace()) {
		if(httpMessage.stacktrace) {
			outputContent += "Stacktrace:\n"
					"<br>\n";

			std::stringstream sstream;
			httpMessage.stacktrace->dump(sstream);
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

    return outputContent;
}

std::string ExceptionHandler::getTextContent() const {
	std::string content;

	content = "jerry/0.1.0: " + httpMessage.title + "\n";
	content += "Status code: " + std::to_string(httpMessage.statusCode) + "\n";

	/* print excpetion message, if enabled */
	if(getShowException()) {
		content += "\n\n";
		content += "Exception: " + httpMessage.message;

		if(!httpMessage.details.empty()) {
			content += "\n\nException details:\n";
			content += httpMessage.details + "\n";
		}
	}

	/* print stacktrace, if enabled */
	if(getShowStacktrace()) {
		content += "\n\n";
		if(httpMessage.stacktrace) {
			content += "Stacktrace:\n";

			std::stringstream sstream;
			httpMessage.stacktrace->dump(sstream);
			content += sstream.str();
		}
		else {
			content += "Stacktrace: not available\n";
		}
	}

	return content;
}

} /* namespace server */
} /* namespace http */
} /* namespace engine */
} /* namespace jerry */
