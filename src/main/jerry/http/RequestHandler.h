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

#ifndef JERRY_HTTP_REQUESTHANDLER_H_
#define JERRY_HTTP_REQUESTHANDLER_H_

#include <esl/http/server/RequestHandler.h>
#include <esl/http/server/RequestContext.h>
#include <esl/object/parameter/Interface.h>
#include <jerry/engine/Endpoint.h>
#include <jerry/http/RequestContext.h>
#include <string>
#include <map>
#include <memory>

namespace jerry {
namespace http {

class RequestHandler : public esl::http::server::RequestHandler {
public:
	RequestHandler(std::unique_ptr<esl::http::server::RequestHandler> requestHandler, std::unique_ptr<esl::http::server::RequestContext> requestContext, const engine::Endpoint& engineEndpoint);

	bool process(const char* contentData, std::size_t contentDataSize) override;

private:
	std::unique_ptr<esl::http::server::RequestHandler> requestHandler;
	std::unique_ptr<esl::http::server::RequestContext> requestContext;
	const engine::Endpoint& engineEndpoint;
};

} /* namespace http */
} /* namespace jerry */

#endif /* JERRY_HTTP_REQUESTHANDLER_H_ */
