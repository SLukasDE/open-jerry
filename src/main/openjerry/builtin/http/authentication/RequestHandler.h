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

#ifndef OPENJERRY_BUILTIN_HTTP_AUTHENTICATION_REQUESTHANDLER_H_
#define OPENJERRY_BUILTIN_HTTP_AUTHENTICATION_REQUESTHANDLER_H_

#include <esl/com/http/server/RequestContext.h>
#include <esl/com/http/server/RequestHandler.h>
#include <esl/io/Input.h>
#include <esl/object/Context.h>
#include <esl/object/InitializeContext.h>
#include <esl/object/Procedure.h>
#include <esl/object/Value.h>

#include <map>
#include <memory>
#include <set>
#include <string>
#include <utility>
#include <vector>

namespace openjerry {
namespace builtin {
namespace http {
namespace authentication {

class RequestHandler final : public virtual esl::com::http::server::RequestHandler, public esl::object::InitializeContext {
public:
	static std::unique_ptr<esl::com::http::server::RequestHandler> createRequestHandler(const std::vector<std::pair<std::string, std::string>>& settings);

	RequestHandler(const std::vector<std::pair<std::string, std::string>>& settings);

	esl::io::Input accept(esl::com::http::server::RequestContext& requestContext) const override;

	void initializeContext(esl::object::Context& objectContext) override;

private:
	using Properties = esl::object::Value<std::map<std::string, std::string>>;
	//using Settings = std::map<std::string, std::string>;

	std::set<std::string> authenticationProceduresId;
	std::vector<esl::object::Procedure*> authenticationProcedures;

	enum {
		reply,
		proceed,
		disconnect
	} behavior = reply;

	enum Allow {
		basic,
		bearer,
		cookie
	};
	std::set<Allow> allows;

	void processRequest(esl::com::http::server::RequestContext& requestContext) const;
	void processRequestBasicAuth(std::set<std::string>& authDataTypes, std::map<std::string, std::string>& settings, esl::com::http::server::RequestContext& requestContext) const;
	void processRequestJWT(std::set<std::string>& authDataTypes, std::map<std::string, std::string>& settings, esl::com::http::server::RequestContext& requestContext) const;

	void processIdentify(esl::object::Context& objectContext) const;

	esl::io::Input processResponse(esl::com::http::server::RequestContext& requestContext) const;
};

} /* namespace authentication */
} /* namespace http */
} /* namespace builtin */
} /* namespace openjerry */

#endif /* OPENJERRY_BUILTIN_HTTP_AUTHENTICATION_REQUESTHANDLER_H_ */
