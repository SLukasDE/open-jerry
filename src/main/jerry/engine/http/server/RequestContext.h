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

#ifndef JERRY_ENGINE_HTTP_SERVER_REQUESTCONTEXT_H_
#define JERRY_ENGINE_HTTP_SERVER_REQUESTCONTEXT_H_

#include <jerry/engine/http/server/Connection.h>
#include <jerry/engine/http/server/Context.h>

#include <esl/com/http/server/RequestContext.h>
#include <esl/com/http/server/Connection.h>
#include <esl/com/http/server/Request.h>
#include <esl/object/Interface.h>
#include <esl/object/ObjectContext.h>

#include <string>

namespace jerry {
namespace engine {
namespace http {
namespace server {

class RequestContext : public esl::com::http::server::RequestContext {
public:
	RequestContext(esl::com::http::server::RequestContext& requestContext);

	esl::com::http::server::Connection& getConnection() const override;
	const esl::com::http::server::Request& getRequest() const override;
	void setPath(std::string path);
	const std::string& getPath() const override;

	esl::object::Interface::ObjectContext& getObjectContext();

	const Context& getHeadersAndErrorHandlingContext() const;
	void setParentHeadersAndErrorHandlingContext(Context* errorHandlingContext);

private:
	esl::com::http::server::RequestContext& requestContext;
	Connection connection;
	std::string path;

	esl::object::ObjectContext objectContext;
	Context headersAndErrorHandlingContext;
};

} /* namespace server */
} /* namespace http */
} /* namespace engine */
} /* namespace jerry */

#endif /* JERRY_ENGINE_HTTP_SERVER_REQUESTCONTEXT_H_ */
