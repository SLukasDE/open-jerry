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

#ifndef JERRY_ENGINE_HTTP_SERVER_REQUESTHANDLER_H_
#define JERRY_ENGINE_HTTP_SERVER_REQUESTHANDLER_H_

#include <esl/com/http/server/requesthandler/Interface.h>
#include <esl/com/http/server/RequestContext.h>
#include <esl/io/Input.h>
#include <esl/object/Interface.h>

namespace jerry {
namespace engine {
namespace http {
namespace server {

class Socket;

class RequestHandler final : public esl::com::http::server::requesthandler::Interface::RequestHandler {
public:
	RequestHandler(Socket& socket);

	esl::io::Input accept(esl::com::http::server::RequestContext& requestContext, esl::object::Interface::ObjectContext& objectContext) const override;

private:
	Socket& socket;
};

} /* namespace server */
} /* namespace http */
} /* namespace engine */
} /* namespace jerry */

#endif /* JERRY_ENGINE_HTTP_SERVER_REQUESTHANDLER_H_ */
