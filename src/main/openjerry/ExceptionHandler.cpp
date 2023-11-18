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

#include <openjerry/ExceptionHandler.h>
#include <openjerry/http/StatusCode.h>
#include <openjerry/Logger.h>

#include <esl/io/FilePosition.h>
#include <esl/database/Diagnostics.h>
#include <esl/plugin/Registry.h>
#include <esl/system/Stacktrace.h>

#include <stdexcept>
#include <memory>
#include <sstream>

namespace openjerry {

namespace {
Logger logger("openjerry::engine::ExceptionHandler");

std::string createFileMessage(const esl::io::FilePosition* filePosition) {
	return filePosition ? "file \"" + filePosition->getFileName() + "\", line " + std::to_string(filePosition->getLineNo()) : "";
}

std::string createStackstrace(const esl::system::Stacktrace* stacktrace) {
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

	if(!plainDetails.empty()) {
		stream << "Details   : " << plainDetails;
		if(plainDetails.at(plainDetails.size()-1) != '\n') {
			stream << "\n";
		}
	}

	if(!fileMessage.empty()) {
		stream << "File      : " << fileMessage;
		if(fileMessage.at(fileMessage.size()-1) != '\n') {
			stream << "\n";
		}
	}

	if(stacktrace.empty()) {
		stream << "Stacktrace: not available\n";
	}
	else {
		stream << "Stacktrace: " << stacktrace << "\n";
	}
}

void ExceptionHandler::dump(esl::monitoring::Streams::Real& stream, esl::monitoring::Streams::Location location) const {
	initialize();

	stream(location.object, location.function, location.file, location.line) << "Exception : " << plainException << "\n";
	stream(location.object, location.function, location.file, location.line) << "What      : " << plainWhat << "\n";

	if(!plainDetails.empty()) {
		stream(location.object, location.function, location.file, location.line) << "Details   : " << plainDetails;
		if(plainDetails.at(plainDetails.size()-1) != '\n') {
			stream(location.object, location.function, location.file, location.line) << "\n";
		}
	}

	if(!fileMessage.empty()) {
		stream(location.object, location.function, location.file, location.line) << "File     : " << fileMessage;
		if(fileMessage.at(fileMessage.size()-1) != '\n') {
			stream(location.object, location.function, location.file, location.line) << "\n";
		}
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
	doInitialize(exceptionPointer);
}

void ExceptionHandler::doInitialize(std::exception_ptr exceptionPointer) const {
	try {
		std::rethrow_exception(exceptionPointer);
    }
	catch(const esl::com::http::server::exception::StatusCode& e) {
		initializeMessage(e);
	}
	catch(const esl::database::exception::SqlError& e) {
		initializeMessage(e);
	}
	catch(const esl::plugin::exception::PluginNotFound& e) {
		initializeMessage(e);
	}
    catch(const std::runtime_error& e) {
    	initializeMessage(e, "std::runtime_error");
    }
    catch(std::out_of_range& e) {
    	initializeMessage(e, "std::out_of_range");
    }
    catch(std::invalid_argument& e) {
    	initializeMessage(e, "std::invalid_argument");
    }
    catch(std::logic_error& e) {
    	initializeMessage(e, "std::logic_error");
    }
    catch(const std::exception& e) {
    	initializeMessage(e, "std::exception");
    }
    catch (...) {
    	initializeMessage();
    }
}

void ExceptionHandler::initializeMessage() const {
	plainException = "unknown exception";
}

void ExceptionHandler::initializeMessage(const esl::com::http::server::exception::StatusCode& e) const {
	stacktrace = createStackstrace(esl::system::Stacktrace::get(e));
	fileMessage = createFileMessage(esl::io::FilePosition::get(e));

	plainException = "esl::com::http::server::exception::StatusCode " + std::to_string(e.getStatusCode());
	if(e.what() && std::string(e.what()) != esl::com::http::server::exception::StatusCode::getMessage(e.getStatusCode())) {
		plainWhat = e.what();
	}
	else {
		plainWhat = http::StatusCode::getMessage(e.getStatusCode());
	}
}

void ExceptionHandler::initializeMessage(const esl::database::exception::SqlError& e) const {
	stacktrace = createStackstrace(esl::system::Stacktrace::get(e));
	fileMessage = createFileMessage(esl::io::FilePosition::get(e));

	plainException = "esl::database::exception::SqlError";
	plainWhat = e.what();

	std::stringstream s;
	s << "SQL Return Code:" << e.getSqlReturnCode() << "\n";
	const esl::database::Diagnostics& diagnostics = e.getDiagnostics();
	diagnostics.dump(s);
	plainDetails += s.str();
}

void ExceptionHandler::initializeMessage(const esl::plugin::exception::PluginNotFound& e) const {
	stacktrace = createStackstrace(esl::system::Stacktrace::get(e));
	fileMessage = createFileMessage(esl::io::FilePosition::get(e));

	plainException = "esl::plugin::exception::PluginNotFound";
	plainWhat = e.what();

	const esl::plugin::Registry::BasePlugins& basePlugins = esl::plugin::Registry::get().getPlugins(e.getTypeIndex());
	if(basePlugins.empty()) {
		plainDetails = "No implementations available.\n";
	}
	else {
		plainDetails = "Implementations available:\n";
		for(const auto& basePlugin : basePlugins) {
			plainDetails += "- " + basePlugin.first + "\n";
		}
	}
}
/*
void ExceptionHandler::initializeMessage(const std::runtime_error& e) const {
	stacktrace = createStackstrace(esl::system::Stacktrace::get(e));

	plainException = "std::runtime_error";
	plainWhat = e.what();
}
*/
void ExceptionHandler::initializeMessage(const std::exception& e, const std::string& aPlainException) const {
	stacktrace = createStackstrace(esl::system::Stacktrace::get(e));
	fileMessage = createFileMessage(esl::io::FilePosition::get(e));

	plainException = aPlainException;
	//plainException = "std::exception";
	plainWhat = e.what();
}

const std::string& ExceptionHandler::getStacktrace() const noexcept {
	return stacktrace;
}

const std::string& ExceptionHandler::getPlainWhat() const noexcept {
	return plainWhat;
}

const std::string& ExceptionHandler::getDetails() const noexcept {
	return plainDetails;
}

} /* namespace openjerry */
