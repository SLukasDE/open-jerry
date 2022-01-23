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

#ifndef JERRY_BUILTIN_HTTP_AUTHENTICATION_REQUESTHANDLER_H_
#define JERRY_BUILTIN_HTTP_AUTHENTICATION_REQUESTHANDLER_H_

#include <esl/com/http/server/RequestContext.h>
#include <esl/com/http/server/requesthandler/Interface.h>
#include <esl/io/Input.h>
#include <esl/module/Interface.h>
#include <esl/object/InitializeContext.h>
#include <esl/object/ObjectContext.h>
#include <esl/object/Value.h>
#include <esl/processing/procedure/Interface.h>

#include <map>
#include <string>
#include <set>
#include <vector>
#include <memory>

namespace jerry {
namespace builtin {
namespace http {
namespace authentication {

class RequestHandler final : public virtual esl::com::http::server::requesthandler::Interface::RequestHandler, public esl::object::InitializeContext {
public:
	static inline const char* getImplementation() {
		return "jerry/authentication";
	}

	static std::unique_ptr<esl::com::http::server::requesthandler::Interface::RequestHandler> createRequestHandler(const esl::module::Interface::Settings& settings);

	RequestHandler(const esl::module::Interface::Settings& settings);

	esl::io::Input accept(esl::com::http::server::RequestContext& requestContext) const override;

	void initializeContext(esl::object::ObjectContext& objectContext) override;

private:
	using Properties = esl::object::Value<std::map<std::string, std::string>>;
	using Settings = std::map<std::string, std::string>;

	std::set<std::string> authenticationProceduresId;
	std::vector<esl::processing::procedure::Interface::Procedure*> authenticationProcedures;

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
	void processRequestBasicAuth(std::set<std::string>& authDataTypes, Settings& settings, esl::com::http::server::RequestContext& requestContext) const;
	void processRequestJWT(std::set<std::string>& authDataTypes, Settings& settings, esl::com::http::server::RequestContext& requestContext) const;

	void processIdentify(esl::object::ObjectContext& objectContext) const;

	esl::io::Input processResponse(esl::com::http::server::RequestContext& requestContext) const;
};

} /* namespace authentication */
} /* namespace http */
} /* namespace builtin */
} /* namespace jerry */

#endif /* JERRY_BUILTIN_HTTP_AUTHENTICATION_REQUESTHANDLER_H_ */