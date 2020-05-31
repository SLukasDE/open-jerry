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
//#include <esl/utility/String.h>
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

bool ExceptionHandler::call(std::function<void()> callFunction, esl::http::server::Connection& connection) {
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

	/* **************** *
	 * Output on logger *
	 * **************** */
    dump(logger.warn);

	/* ************* *
	* HTTP Response *
	* ************* */
    dump(connection);

	return true;
}

void ExceptionHandler::dump(std::ostream& stream) const {
	stream << message.title << "\n";
	stream << message.message << "\n";
	if(!message.details.empty()) {
		stream << "\n\n" << message.details << "\n";
	}

	if(message.stacktrace) {
		stream << "Stacktrace:\n";
		message.stacktrace->dump(logger.warn);
	}
	else {
		stream << "Stacktrace: not available\n";
	}
}

void ExceptionHandler::dump(esl::logging::StreamReal& stream, esl::logging::Location location) const {
	stream(location.object, location.function, location.file, location.line) << message.title << "\n";
	stream(location.object, location.function, location.file, location.line) << message.message << "\n";
	if(!message.details.empty()) {
		stream(location.object, location.function, location.file, location.line) << "\n\n" << message.details << "\n";
	}

	if(message.stacktrace) {
		stream(location.object, location.function, location.file, location.line) << "Stacktrace:\n";
		message.stacktrace->dump(stream, location);
	}
	else {
		stream(location.object, location.function, location.file, location.line) << "Stacktrace: not available\n";
	}
}

void ExceptionHandler::dump(esl::http::server::Connection& connection, std::function<const Document*(unsigned short statusCode)> findDocument) const {
	const Document* errorDocument = nullptr;
	if(findDocument) {
		errorDocument = findDocument(message.statusCode);
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
				std::unique_ptr<esl::http::server::ResponseFile> response(new esl::http::server::ResponseFile(message.statusCode, mime, url.getPath()));
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
	if(message.contentType == esl::utility::MIME::textHtml) {
	    content = getHTMLContent();
	}
	else if(message.contentType == esl::utility::MIME::textPlain) {
	    content = getTextContent();
	}
	else {
		content = message.message;
	}

	std::unique_ptr<esl::http::server::ResponseDynamic> response;
	response.reset(new esl::http::server::ResponseDynamic(message.statusCode, message.contentType, std::move(content)));
	connection.sendResponse(std::move(response));
}

void ExceptionHandler::setMessage() {
	message.statusCode = 500;
	message.contentType = esl::utility::MIME(esl::utility::MIME::textHtml);

	message.title = "Unknown Exception Error";

	message.message = "unknown exception";

	message.details.clear();

	message.stacktrace.reset();
}

void ExceptionHandler::setMessage(const esl::http::server::exception::StatusCode& e) {
	message.statusCode = e.getStatusCode();
	message.contentType = e.getMimeType();
	message.title = std::to_string(e.getStatusCode()) + " " + http::StatusCode::getMessage(e.getStatusCode());

	if(e.what() && std::string(e.what()) != esl::http::server::exception::StatusCode::getMessage(e.getStatusCode())) {
		message.message = e.what();
	}
	else {
		message.message = http::StatusCode::getMessage(e.getStatusCode());
	}

	message.details.clear();
}

void ExceptionHandler::setMessage(const esl::database::exception::SqlError& e) {
	message.statusCode = 500;
	message.contentType = esl::utility::MIME(esl::utility::MIME::textHtml);

	message.title = "SQL Error";

	message.message = e.what();

	std::stringstream s;
	s << "SQL Return Code:" << e.getSqlReturnCode() << "\n";
	const esl::database::Diagnostics& diagnostics = e.getDiagnostics();
	diagnostics.dump(s);
	message.details = s.str();

	message.stacktrace = createStackstrace(esl::getStacktrace(e));
}

void ExceptionHandler::setMessage(const std::runtime_error& e) {
	message.statusCode = 500;
	message.contentType = esl::utility::MIME(esl::utility::MIME::textHtml);

	message.title = "std::runtime_error";

	message.message = e.what();

	message.details.clear();

	message.stacktrace = createStackstrace(esl::getStacktrace(e));
}

void ExceptionHandler::setMessage(const std::exception& e) {
	for(auto exceptionMessageInterface : exceptionMessageInterfaces) {
		//std::unique_ptr<esl::http::server::exception::Interface::Message> messagePtr;
		auto messagePtr = exceptionMessageInterface.get().createMessage(e);
		if(messagePtr) {
			message = std::move(*messagePtr);
			return;
		}
	}

	message.statusCode = 500;
	message.contentType = esl::utility::MIME(esl::utility::MIME::textHtml);

	message.title = "std::exception";

	message.message = e.what();

	message.details.clear();

	message.stacktrace = createStackstrace(esl::getStacktrace(e));
}

std::string ExceptionHandler::getHTMLContent() const {
	std::string outputContent;

	outputContent = "<!DOCTYPE html>\n"
			"<html>\n"
			"<head>\n";

	outputContent += "<title>" + html::toHTML(message.title) + "</title>\n";
	outputContent += "<body bgcolor=\"white\">\n";

	/* print excpetion message, if enabled */
	if(showException) {
		outputContent += "<center><h1>\n";
		outputContent += html::toHTML(message.message) + "\n";
		outputContent += "</h1></center>\n";

		if(!message.details.empty()) {
			outputContent += "<hr>\n";
			outputContent += html::toHTML(message.details) + "<br>\n";
		}
	}

	/* print stacktrace, if enabled */
	if(showStacktrace) {
		if(message.stacktrace) {
			outputContent += "Stacktrace:\n"
					"<br>\n";

			std::stringstream sstream;
			message.stacktrace->dump(sstream);
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

	content = "jerry/0.1.0: " + message.title + "\n";
	content += "Status code: " + std::to_string(message.statusCode) + "\n";

	/* print excpetion message, if enabled */
	if(showException) {
		content += "\n\n";
		content += "Exception: " + message.message;

		if(!message.details.empty()) {
			content += "\n\nException details:\n";
			content += message.details + "\n";
		}
	}

	/* print stacktrace, if enabled */
	if(showStacktrace) {
		content += "\n\n";
		if(message.stacktrace) {
			content += "Stacktrace:\n";

			std::stringstream sstream;
			message.stacktrace->dump(sstream);
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
