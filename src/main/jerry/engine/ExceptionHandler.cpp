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
#include <jerry/http/StatusCode.h>
#include <jerry/Logger.h>

#include <esl/Stacktrace.h>

#include <sstream>
#include <vector>

namespace jerry {
namespace engine {

namespace {
Logger logger("jerry::engine::ExceptionHandler");

std::unique_ptr<esl::Stacktrace> createStackstrace(const esl::Stacktrace* stacktracePtr) {
	if(stacktracePtr) {
		return std::unique_ptr<esl::Stacktrace>(new esl::Stacktrace(*stacktracePtr));
	}
	return nullptr;
}
} /* anonymous namespace */

void ExceptionHandler::setShowException(bool aShowException) {
	showException = aShowException;
}

bool ExceptionHandler::getShowException() const noexcept {
	return showException;
}

void ExceptionHandler::setShowStacktrace(bool aShowStacktrace) {
	showStacktrace = aShowStacktrace;
}

bool ExceptionHandler::getShowStacktrace() const noexcept {
	return showStacktrace;
}

bool ExceptionHandler::call(std::function<void()> callFunction) {
    try {
    	callFunction();
    	return false;
    }
	catch(const esl::com::http::server::exception::StatusCode& e) {
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

const ExceptionMessage& ExceptionHandler::getMessage() const {
	return plainMessage;
}

void ExceptionHandler::setMessage() {
	plainMessage.title = "Unknown Exception Error";
	plainMessage.message = "unknown exception";
	plainMessage.stacktrace.reset();
}

void ExceptionHandler::setMessage(const esl::com::http::server::exception::StatusCode& e) {
	plainMessage.title = "HTTP status code " + std::to_string(e.getStatusCode());
	if(e.what() && std::string(e.what()) != esl::com::http::server::exception::StatusCode::getMessage(e.getStatusCode())) {
		plainMessage.message = e.what();
	}
	else {
		plainMessage.message = jerry::http::StatusCode::getMessage(e.getStatusCode());
	}
	plainMessage.stacktrace = createStackstrace(esl::getStacktrace(e));
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
}

void ExceptionHandler::setMessage(const std::runtime_error& e) {
	plainMessage.title = "std::runtime_error";
	plainMessage.message = e.what();
	plainMessage.stacktrace = createStackstrace(esl::getStacktrace(e));
}

void ExceptionHandler::setMessage(const std::exception& e) {
	plainMessage.title = "std::exception";
	plainMessage.message = e.what();
	plainMessage.stacktrace = createStackstrace(esl::getStacktrace(e));
}

} /* namespace engine */
} /* namespace jerry */
