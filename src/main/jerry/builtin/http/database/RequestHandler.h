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

#ifndef JERRY_BUILTIN_HTTP_DATABASE_REQUESTHANDLER_H_
#define JERRY_BUILTIN_HTTP_DATABASE_REQUESTHANDLER_H_

#include <esl/com/http/server/RequestContext.h>
#include <esl/com/http/server/RequestHandler.h>
#include <esl/database/Connection.h>
#include <esl/database/ConnectionFactory.h>
#include <esl/io/Input.h>
#include <esl/object/Context.h>
#include <esl/object/InitializeContext.h>

#include <memory>
#include <string>
#include <utility>
#include <vector>

namespace jerry {
namespace builtin {
namespace http {
namespace database {

class RequestHandler final : public virtual esl::com::http::server::RequestHandler, public esl::object::InitializeContext {
public:
	static inline const char* getImplementation() {
		return "jerry/database";
	}

	static std::unique_ptr<esl::com::http::server::RequestHandler> createRequestHandler(const std::vector<std::pair<std::string, std::string>>& settings);

	RequestHandler(const std::vector<std::pair<std::string, std::string>>& settings);

	esl::io::Input accept(esl::com::http::server::RequestContext& requestContext) const override;

	void initializeContext(esl::object::Context& objectContext) override;

private:
	std::string connectionId;
	std::string sql;
	esl::database::ConnectionFactory* connectionFactory = nullptr;
};

} /* namespace database */
} /* namespace http */
} /* namespace builtin */
} /* namespace jerry */

#endif /* JERRY_BUILTIN_HTTP_DATABASE_REQUESTHANDLER_H_ */
