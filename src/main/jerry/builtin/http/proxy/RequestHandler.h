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

#ifndef JERRY_BUILTIN_HTTP_PROXY_REQUESTHANDLER_H_
#define JERRY_BUILTIN_HTTP_PROXY_REQUESTHANDLER_H_

#include <esl/com/http/server/RequestContext.h>
#include <esl/com/http/server/requesthandler/Interface.h>
#include <esl/com/http/client/Interface.h>
#include <esl/io/Input.h>
#include <esl/object/Interface.h>
#include <esl/object/InitializeContext.h>

#include <string>
#include <memory>

namespace jerry {
namespace builtin {
namespace http {
namespace proxy {

class RequestHandler final : public virtual esl::com::http::server::requesthandler::Interface::RequestHandler, public esl::object::InitializeContext {
public:
	static inline const char* getImplementation() {
		return "jerry/builtin/http/proxy";
	}

	static std::unique_ptr<esl::com::http::server::requesthandler::Interface::RequestHandler> createRequestHandler(const esl::object::Interface::Settings& settings);

	RequestHandler(const esl::object::Interface::Settings& settings);

	esl::io::Input accept(esl::com::http::server::RequestContext& requestContext, esl::object::Interface::ObjectContext& objectContext) const override;

	void initializeContext(esl::object::Interface::ObjectContext& objectContext) override;

private:
	std::string httpClientId;
	esl::com::http::client::Interface::ConnectionFactory* connectionFactory = nullptr;
};

} /* namespace proxy */
} /* namespace http */
} /* namespace builtin */
} /* namespace jerry */

#endif /* JERRY_BUILTIN_HTTP_PROXY_REQUESTHANDLER_H_ */