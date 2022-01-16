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

#ifndef JERRY_ENGINE_HTTP_EXCEPTIONHANDLER_H_
#define JERRY_ENGINE_HTTP_EXCEPTIONHANDLER_H_

#include <jerry/ExceptionHandler.h>
#include <jerry/engine/http/RequestContext.h>
#include <jerry/engine/http/Document.h>
#include <jerry/engine/http/Context.h>
#include <jerry/utility/MIME.h>

#include <esl/com/http/server/Connection.h>
#include <esl/com/http/server/exception/StatusCode.h>
#include <esl/database/exception/SqlError.h>

#include <exception>
#include <stdexcept>

namespace jerry {
namespace engine {
namespace http {


class ExceptionHandler : public jerry::ExceptionHandler {
public:
	ExceptionHandler(std::exception_ptr e);

	void dumpHttp(esl::com::http::server::Connection& connection, const Context* errorHandlingContext) const;

protected:
	void initializeMessage() const override;
	void initializeMessage(const esl::com::http::server::exception::StatusCode& e) const override;
	void initializeMessage(const esl::database::exception::SqlError& e) const override;
	void initializeMessage(const std::runtime_error& e) const override;
	void initializeMessage(const std::exception& e) const override;

private:
	std::string getHTMLContent(bool showException, bool showStacktrace) const;
	std::string getTextContent(bool showException, bool showStacktrace) const;

	//std::function<const http::Document*(unsigned short statusCode)> findDocument;

	mutable unsigned short httpStatusCode = 500;
	mutable esl::utility::MIME httpContentType = esl::utility::MIME::textHtml;
	mutable std::string httpTitle;
	mutable std::string httpMessage;
};


} /* namespace http */
} /* namespace engine */
} /* namespace jerry */

#endif /* JERRY_ENGINE_HTTP_EXCEPTIONHANDLER_H_ */
