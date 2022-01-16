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

#include <jerry/ExceptionHandler.h>
#include <jerry/http/StatusCode.h>
#include <jerry/Logger.h>

#include <esl/Stacktrace.h>

#include <memory>
#include <ostream>
#include <sstream>

namespace jerry {

namespace {
Logger logger("jerry::engine::ExceptionHandler");

std::string createStackstrace(const esl::Stacktrace* stacktrace) {
	std::stringstream sstream;
	if(stacktrace) {
		stacktrace->dump(sstream);
	}
	return sstream.str();
}
} /* anonymous namespace */

ExceptionHandler::ExceptionHandler(std::exception_ptr aExceptionPointer)
: exceptionPointer(aExceptionPointer)
{ }

void ExceptionHandler::dump(std::ostream& stream) const {
	initialize();

	stream << "Exception : " << plainException << "\n";
	stream << "What      : " << plainWhat << "\n";
	if(plainDetails.empty() == false) {
		stream << "Details   : " << plainDetails << "\n";
	}

	if(stacktrace.empty()) {
		stream << "Stacktrace: not available\n";
	}
	else {
		stream << "Stacktrace: " << stacktrace << "\n";
	}
}

void ExceptionHandler::dump(esl::logging::StreamReal& stream, esl::logging::Location location) const {
	initialize();

	stream(location.object, location.function, location.file, location.line) << "Exception : " << plainException << "\n";
	stream(location.object, location.function, location.file, location.line) << "What      : " << plainWhat << "\n";
	if(plainDetails.empty() == false) {
		stream(location.object, location.function, location.file, location.line) << "Details   : " << plainDetails << "\n";
	}

	if(stacktrace.empty()) {
		stream(location.object, location.function, location.file, location.line) << "Stacktrace: not available\n";
	}
	else {
		stream(location.object, location.function, location.file, location.line) << "Stacktrace:" << stacktrace << "\n";
	}
}

void ExceptionHandler::initialize() const {
	if(isInitialized) {
		return;
	}
	isInitialized = true;

	try {
		std::rethrow_exception(exceptionPointer);
    }
	catch(const esl::com::http::server::exception::StatusCode& e) {
		initializeMessage(e);
	}
	catch(const esl::database::exception::SqlError& e) {
		initializeMessage(e);
	}
    catch(const std::runtime_error& e) {
    	initializeMessage(e);
    }
    catch(const std::exception& e) {
    	initializeMessage(e);
    }
    catch (...) {
    	initializeMessage();
    }
}

void ExceptionHandler::initializeMessage() const {
	plainException = "unknown exception";
}

void ExceptionHandler::initializeMessage(const esl::com::http::server::exception::StatusCode& e) const {
	stacktrace = createStackstrace(esl::getStacktrace(e));

	plainException = "esl::com::http::server::exception::StatusCode " + std::to_string(e.getStatusCode());
	if(e.what() && std::string(e.what()) != esl::com::http::server::exception::StatusCode::getMessage(e.getStatusCode())) {
		plainWhat = e.what();
	}
	else {
		plainWhat = http::StatusCode::getMessage(e.getStatusCode());
	}
}

void ExceptionHandler::initializeMessage(const esl::database::exception::SqlError& e) const {
	stacktrace = createStackstrace(esl::getStacktrace(e));

	plainException = "esl::database::exception::SqlError";
	plainWhat = e.what();

	std::stringstream s;
	s << "SQL Return Code:" << e.getSqlReturnCode() << "\n";
	const esl::database::Diagnostics& diagnostics = e.getDiagnostics();
	diagnostics.dump(s);
	plainDetails += s.str();

	stacktrace = createStackstrace(esl::getStacktrace(e));
}

void ExceptionHandler::initializeMessage(const std::runtime_error& e) const {
	stacktrace = createStackstrace(esl::getStacktrace(e));

	plainException = "std::runtime_error";
	plainWhat = e.what();
}

void ExceptionHandler::initializeMessage(const std::exception& e) const {
	stacktrace = createStackstrace(esl::getStacktrace(e));

	plainException = "std::exception";
	plainWhat = e.what();
}

const std::string& ExceptionHandler::getStacktrace() const {
	return stacktrace;
}

const std::string& ExceptionHandler::getDetails() const {
	return plainDetails;
}

} /* namespace jerry */
