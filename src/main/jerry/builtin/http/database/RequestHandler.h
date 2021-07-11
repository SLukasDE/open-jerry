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

#ifndef JERRY_BUILTIN_HTTP_DATABASE_REQUESTHANDLER_H_
#define JERRY_BUILTIN_HTTP_DATABASE_REQUESTHANDLER_H_

#include <esl/io/Input.h>
#include <esl/com/http/server/RequestContext.h>
#include <esl/object/Interface.h>

#include <memory>

namespace jerry {
namespace builtin {
namespace http {
namespace database {

struct RequestHandler final {
	RequestHandler() = delete;

	static esl::io::Input createRequestHandler(esl::com::http::server::RequestContext& requestContext);
	static std::unique_ptr<esl::object::Interface::Object> createSettings(const esl::object::Interface::Settings& settings);

	static inline const char* getImplementation() {
		return "jerry/builtin/http/database";
	}
};

} /* namespace database */
} /* namespace http */
} /* namespace builtin */
} /* namespace jerry */

#endif /* JERRY_BUILTIN_HTTP_DATABASE_REQUESTHANDLER_H_ */
