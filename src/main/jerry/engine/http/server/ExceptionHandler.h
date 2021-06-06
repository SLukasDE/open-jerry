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

#ifndef JERRY_ENGINE_HTTP_SERVER_EXCEPTIONHANDLER_H_
#define JERRY_ENGINE_HTTP_SERVER_EXCEPTIONHANDLER_H_

#include <jerry/engine/ExceptionHandler.h>
#include <jerry/engine/ExceptionMessage.h>
#include <jerry/engine/http/server/Document.h>

#include <esl/com/http/server/Connection.h>
#include <esl/com/http/server/exception/StatusCode.h>
#include <esl/com/http/server/exception/Interface.h>
#include <esl/database/exception/SqlError.h>

#include <stdexcept>
#include <functional>

namespace jerry {
namespace engine {
namespace http {
namespace server {

class ExceptionHandler : public engine::ExceptionHandler {
public:
	void dump(esl::com::http::server::Connection& connection, std::function<const http::server::Document*(unsigned short statusCode)> findDocument = nullptr) const;

protected:
	void setMessage() override;
	void setMessage(const esl::com::http::server::exception::StatusCode& e) override;
	void setMessage(const esl::database::exception::SqlError& e) override;
	void setMessage(const std::runtime_error& e) override;
	void setMessage(const std::exception& e) override;

private:
	std::string getHTMLContent() const;
	std::string getTextContent() const;

	esl::com::http::server::exception::Interface::Message httpMessage;
};

} /* namespace server */
} /* namespace http */
} /* namespace engine */
} /* namespace jerry */

#endif /* JERRY_ENGINE_HTTP_SERVER_EXCEPTIONHANDLER_H_ */
