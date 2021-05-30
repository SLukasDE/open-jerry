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

#ifndef JERRY_ENGINE_EXCEPTIONHANDLER_H_
#define JERRY_ENGINE_EXCEPTIONHANDLER_H_

#include <jerry/engine/ExceptionMessage.h>

#include <esl/http/server/exception/StatusCode.h>
#include <esl/database/exception/SqlError.h>
#include <esl/logging/StreamReal.h>
#include <esl/logging/StreamEmpty.h>
#include <esl/logging/Location.h>

#include <string>
#include <stdexcept>
#include <functional>
#include <ostream>

namespace jerry {
namespace engine {

class ExceptionHandler {
public:
	virtual ~ExceptionHandler() = default;

	void setShowException(bool showException);
	bool getShowException() const noexcept;

	void setShowStacktrace(bool showStacktrace);
	bool getShowStacktrace() const noexcept;

	/* return true if exception occurred */
	bool call(std::function<void()> callFunction);
	//bool call(std::function<void()> callFunction, esl::http::server::Connection& connection);

	void dump(std::ostream& stream) const;
	void dump(esl::logging::StreamReal& stream, esl::logging::Location location = esl::logging::Location{}) const;
	inline void dump(esl::logging::StreamEmpty& stream, esl::logging::Location location = esl::logging::Location{}) const { };

protected:
	const ExceptionMessage& getMessage() const;

	virtual void setMessage();
	virtual void setMessage(const esl::http::server::exception::StatusCode& e);
	virtual void setMessage(const esl::database::exception::SqlError& e);
	virtual void setMessage(const std::runtime_error& e);
	virtual void setMessage(const std::exception& e);

private:
	bool showException = false;
	bool showStacktrace = false;

	ExceptionMessage plainMessage;
};

} /* namespace engine */
} /* namespace jerry */

#endif /* JERRY_ENGINE_EXCEPTIONHANDLER_H_ */
