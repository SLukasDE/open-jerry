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

#ifndef JERRY_ENGINE_EXCEPTIONHANDLER_H_
#define JERRY_ENGINE_EXCEPTIONHANDLER_H_

#include <jerry/engine/ExceptionMessage.h>

#include <esl/com/http/server/exception/StatusCode.h>
#include <esl/database/exception/SqlError.h>
#include <esl/logging/StreamReal.h>
#include <esl/logging/StreamEmpty.h>
#include <esl/logging/Location.h>

#include <string>
#include <exception>
#include <stdexcept>

namespace jerry {
namespace engine {

class ExceptionHandler {
public:
	ExceptionHandler(std::exception_ptr exceptionPointer);
	virtual ~ExceptionHandler() = default;

	void dump(std::ostream& stream) const;
	void dump(esl::logging::StreamReal& stream, esl::logging::Location location = esl::logging::Location{}) const;
	inline void dump(esl::logging::StreamEmpty& stream, esl::logging::Location location = esl::logging::Location{}) const { };

protected:
	void initialize() const;

	virtual void initializeMessage() const;
	virtual void initializeMessage(const esl::com::http::server::exception::StatusCode& e) const;
	virtual void initializeMessage(const esl::database::exception::SqlError& e) const;
	virtual void initializeMessage(const std::runtime_error& e) const;
	virtual void initializeMessage(const std::exception& e) const;

	const std::string& getStacktrace() const;
	const std::string& getDetails() const;

private:
	std::exception_ptr exceptionPointer;
	mutable bool isInitialized = false;

	mutable std::string stacktrace;

	mutable std::string plainException;
	mutable std::string plainWhat;
	mutable std::string plainDetails;
};

} /* namespace engine */
} /* namespace jerry */

#endif /* JERRY_ENGINE_EXCEPTIONHANDLER_H_ */
