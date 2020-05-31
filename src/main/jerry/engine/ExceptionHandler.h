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

#ifndef JERRY_ENGINE_EXCEPTIONHANDLER_H_
#define JERRY_ENGINE_EXCEPTIONHANDLER_H_

#include <jerry/engine/Document.h>

#include <esl/http/server/exception/StatusCode.h>
#include <esl/http/server/exception/Interface.h>
#include <esl/database/exception/SqlError.h>
#include <esl/http/server/Connection.h>
//#include <esl/utility/MIME.h>
//#include <esl/Stacktrace.h>
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
	// required to call setMessage(const esl::http::server::exception::StatusCode& e)
	friend class Engine;
	friend class Listener;
public:
	static void initialize();

	void setShowException(bool showException);
	void setShowStacktrace(bool showStacktrace);

	/* return true if exception occurred */
	bool call(std::function<void()> callFunction, esl::http::server::Connection& connection);

	void dump(std::ostream& stream) const;
	void dump(esl::logging::StreamReal& stream, esl::logging::Location location = esl::logging::Location{}) const;
	inline void dump(esl::logging::StreamEmpty& stream, esl::logging::Location location = esl::logging::Location{}) const { };
	void dump(esl::http::server::Connection& connection, std::function<const Document*(unsigned short statusCode)> findDocument = nullptr) const;

private:
	void setMessage();
	void setMessage(const esl::http::server::exception::StatusCode& e);
	void setMessage(const esl::database::exception::SqlError& e);
	void setMessage(const std::runtime_error& e);
	void setMessage(const std::exception& e);

	std::string getHTMLContent() const;
	std::string getTextContent() const;

	bool showException = false;
	bool showStacktrace = false;
	esl::http::server::exception::Interface::Message message;
};

} /* namespace engine */
} /* namespace jerry */

#endif /* JERRY_ENGINE_EXCEPTIONHANDLER_H_ */
