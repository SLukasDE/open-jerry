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

#include <jerry/engine/ExceptionHandler.h>
#include <jerry/html/HTML.h>
#include <jerry/http/StatusCode.h>
#include <jerry/utility/URL.h>
#include <jerry/utility/MIME.h>
#include <jerry/Logger.h>

#include <esl/http/server/ResponseDynamic.h>
#include <esl/http/server/ResponseFile.h>
#include <esl/http/server/ResponseStatic.h>
#include <esl/http/server/exception/StatusCode.h>
#include <esl/database/exception/SqlError.h>

#include <sstream>
#include <vector>

namespace jerry {
namespace engine {

namespace {
Logger logger("jerry::engine::ExceptionHandler");

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

std::vector<std::reference_wrapper<const esl::http::server::exception::Interface>> exceptionMessageInterfaces;

}

void ExceptionHandler::initialize() {
	for(const auto& metaInterface : esl::getModule().getMetaInterfaces()) {
		if(metaInterface.type != esl::http::server::exception::Interface::getType()) {
			continue;
		}

		const esl::http::server::exception::Interface& exceptionMessageInterface = esl::getModule().getInterface<esl::http::server::exception::Interface>(metaInterface.implementation);
		exceptionMessageInterfaces.emplace_back(exceptionMessageInterface);
	}

}

void ExceptionHandler::setShowException(bool aShowException) {
	showException = aShowException;
}

void ExceptionHandler::setShowStacktrace(bool aShowStacktrace) {
	showStacktrace = aShowStacktrace;
}

bool ExceptionHandler::call(std::function<void()> callFunction) {
//bool ExceptionHandler::call(std::function<void()> callFunction, esl::http::server::Connection& connection) {
    try {
    	callFunction();
    	return false;
    }
	catch(const esl::http::server::exception::StatusCode& e) {
    	setMessage(e);
	}
	catch(const esl::database::exception::SqlError& e) {
		setMessage(e);
	}
    catch(const std::runtime_error& e) {
    	setMessage(e);
    }
    catch(const std::exception& e) {
    	setMessage(e);
    }
    catch (...) {
    	setMessage();
    }
#if 0
	/* **************** *
	 * Output on logger *
	 * **************** */
    dump(logger.warn);

	/* ************* *
	* HTTP Response *
	* ************* */
    dump(connection);
#endif

	return true;
}

void ExceptionHandler::dump(std::ostream& stream) const {
	stream << plainMessage.title << "\n";
	stream << plainMessage.message << "\n";

	if(plainMessage.stacktrace) {
		stream << "Stacktrace:\n";
		plainMessage.stacktrace->dump(logger.warn);
	}
	else {
		stream << "Stacktrace: not available\n";
	}
}

void ExceptionHandler::dump(esl::logging::StreamReal& stream, esl::logging::Location location) const {
	stream(location.object, location.function, location.file, location.line) << plainMessage.title << "\n";
	stream(location.object, location.function, location.file, location.line) << plainMessage.message << "\n";

	if(plainMessage.stacktrace) {
		stream(location.object, location.function, location.file, location.line) << "Stacktrace:\n";
		plainMessage.stacktrace->dump(stream, location);
	}
	else {
		stream(location.object, location.function, location.file, location.line) << "Stacktrace: not available\n";
	}
}

void ExceptionHandler::dump(esl::http::server::Connection& connection, std::function<const http::Document*(unsigned short statusCode)> findDocument) const {
	const http::Document* errorDocument = nullptr;
	if(findDocument) {
		errorDocument = findDocument(httpMessage.statusCode);
	}

	if(errorDocument) {
		utility::URL url(errorDocument->getPath());

		if(url.getScheme() == "http" || url.getScheme() == "https") {
			std::unique_ptr<esl::http::server::ResponseStatic> response;
			response.reset(new esl::http::server::ResponseStatic(301, esl::utility::MIME::textHtml, PAGE_301.data(), PAGE_301.size()));
			response->addHeader("Location", errorDocument->getPath());
			connection.sendResponse(std::move(response));

			return;
		}
		if(url.getScheme().empty() || url.getScheme() == "file") {
			/* if we don't need to parse the file, then we are done very quick */
			if(errorDocument->getLanguage().empty()) {
				esl::utility::MIME mime = utility::MIME::byFilename(url.getPath());
				std::unique_ptr<esl::http::server::ResponseFile> response(new esl::http::server::ResponseFile(httpMessage.statusCode, mime, url.getPath()));
				connection.sendResponse(std::move(response));

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

	std::unique_ptr<esl::http::server::ResponseDynamic> response;
	response.reset(new esl::http::server::ResponseDynamic(httpMessage.statusCode, httpMessage.contentType, std::move(content)));
	connection.sendResponse(std::move(response));
}

void ExceptionHandler::setMessage() {
	plainMessage.title = "Unknown Exception Error";
	plainMessage.message = "unknown exception";
	plainMessage.stacktrace.reset();

	httpMessage.statusCode = 500;
	httpMessage.contentType = esl::utility::MIME(esl::utility::MIME::textHtml);
	httpMessage.title = "Unknown Exception Error";
	httpMessage.message = "unknown exception";
	httpMessage.details.clear();
	httpMessage.stacktrace.reset();
}

void ExceptionHandler::setMessage(const esl::http::server::exception::StatusCode& e) {
	plainMessage.title = "HTTP status code " + std::to_string(e.getStatusCode());
	if(e.what() && std::string(e.what()) != esl::http::server::exception::StatusCode::getMessage(e.getStatusCode())) {
		plainMessage.message = e.what();
	}
	else {
		plainMessage.message = jerry::http::StatusCode::getMessage(e.getStatusCode());
	}
	plainMessage.stacktrace = createStackstrace(esl::getStacktrace(e));

	httpMessage.statusCode = e.getStatusCode();
	httpMessage.contentType = e.getMimeType();
	httpMessage.title = std::to_string(e.getStatusCode()) + " " + jerry::http::StatusCode::getMessage(e.getStatusCode());
	if(e.what() && std::string(e.what()) != esl::http::server::exception::StatusCode::getMessage(e.getStatusCode())) {
		httpMessage.message = e.what();
	}
	else {
		httpMessage.message = jerry::http::StatusCode::getMessage(e.getStatusCode());
	}
	httpMessage.details.clear();
	httpMessage.stacktrace = createStackstrace(esl::getStacktrace(e));
}

void ExceptionHandler::setMessage(const esl::database::exception::SqlError& e) {
	std::stringstream s;
	s << "SQL Return Code:" << e.getSqlReturnCode() << "\n";
	const esl::database::Diagnostics& diagnostics = e.getDiagnostics();
	diagnostics.dump(s);

	plainMessage.title = "SQL Error";
	plainMessage.message = e.what();
	plainMessage.message += "\n\n";
	plainMessage.message += s.str();
	plainMessage.stacktrace = createStackstrace(esl::getStacktrace(e));

	httpMessage.statusCode = 500;
	httpMessage.contentType = esl::utility::MIME(esl::utility::MIME::textHtml);
	httpMessage.title = "SQL Error";
	httpMessage.message = e.what();
	httpMessage.details = s.str();
	httpMessage.stacktrace = createStackstrace(esl::getStacktrace(e));
}

void ExceptionHandler::setMessage(const std::runtime_error& e) {
	plainMessage.title = "std::runtime_error";
	plainMessage.message = e.what();
	plainMessage.stacktrace = createStackstrace(esl::getStacktrace(e));

	httpMessage.statusCode = 500;
	httpMessage.contentType = esl::utility::MIME(esl::utility::MIME::textHtml);
	httpMessage.title = "std::runtime_error";
	httpMessage.message = e.what();
	httpMessage.details.clear();
	httpMessage.stacktrace = createStackstrace(esl::getStacktrace(e));
}

void ExceptionHandler::setMessage(const std::exception& e) {
	plainMessage.title = "std::runtime_error";
	plainMessage.message = e.what();
	plainMessage.stacktrace = createStackstrace(esl::getStacktrace(e));

	bool httpMessageFound = false;
	for(auto exceptionMessageInterface : exceptionMessageInterfaces) {
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
	if(showException) {
		outputContent += "<center><h1>\n";
		outputContent += html::toHTML(httpMessage.message) + "\n";
		outputContent += "</h1></center>\n";

		if(!httpMessage.details.empty()) {
			outputContent += "<hr>\n";
			outputContent += html::toHTML(httpMessage.details) + "<br>\n";
		}
	}

	/* print stacktrace, if enabled */
	if(showStacktrace) {
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
	if(showException) {
		content += "\n\n";
		content += "Exception: " + httpMessage.message;

		if(!httpMessage.details.empty()) {
			content += "\n\nException details:\n";
			content += httpMessage.details + "\n";
		}
	}

	/* print stacktrace, if enabled */
	if(showStacktrace) {
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

} /* namespace engine */
} /* namespace jerry */
