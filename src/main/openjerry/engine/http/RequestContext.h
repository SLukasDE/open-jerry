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

#ifndef OPENJERRY_ENGINE_HTTP_REQUESTCONTEXT_H_
#define OPENJERRY_ENGINE_HTTP_REQUESTCONTEXT_H_

#include <openjerry/engine/http/Connection.h>
#include <openjerry/engine/http/Context.h>

#include <esl/com/http/server/RequestContext.h>
#include <esl/com/http/server/Connection.h>
#include <esl/com/http/server/Request.h>
#include <esl/object/Context.h>

#include <string>

namespace openjerry {
namespace engine {
namespace http {


class RequestContext : public esl::com::http::server::RequestContext {
public:
	RequestContext(esl::com::http::server::RequestContext& requestContext);

	esl::com::http::server::Connection& getConnection() const override;
	const esl::com::http::server::Request& getRequest() const override;
	void setPath(std::string path);
	const std::string& getPath() const override;
	esl::object::Context& getObjectContext() override;
	const esl::object::Context& getObjectContext() const override;

	void setHeadersContext(const Context* headersContext);
	const Context* getHeadersContext() const;

	void setErrorHandlingContext(const Context* errorHandlingContext);
	const Context* getErrorHandlingContext() const;

private:
	esl::com::http::server::RequestContext& baseRequestContext;
	Connection connection;
	std::string path;

	const Context* headersContext = nullptr;
	const Context* errorHandlingContext = nullptr;
};


} /* namespace http */
} /* namespace engine */
} /* namespace openjerry */

#endif /* OPENJERRY_ENGINE_HTTP_REQUESTCONTEXT_H_ */
