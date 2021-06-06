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

#ifndef JERRY_BUILTIN_HTTP_FILE_REQUESTHANDLER_H_
#define JERRY_BUILTIN_HTTP_FILE_REQUESTHANDLER_H_

#include <jerry/builtin/http/file/Settings.h>

#include <esl/io/Consumer.h>
#include <esl/io/Input.h>
#include <esl/com/http/server/RequestContext.h>
#include <esl/com/http/server/Request.h>

#include <string>

namespace jerry {
namespace builtin {
namespace http {
namespace file {

class RequestHandler : public esl::io::Consumer {
public:
	static esl::io::Input create(esl::com::http::server::RequestContext& requestContext);

	RequestHandler(esl::com::http::server::RequestContext& requestContext, const Settings& settings);

	/* return: true for every kind of success and get called again for more content data
	 *         false for failure or to get not called again
	 */
	bool consume(esl::io::Reader& reader) override;
};

} /* namespace file */
} /* namespace http */
} /* namespace builtin */
} /* namespace jerry */

#endif /* JERRY_BUILTIN_HTTP_FILE_REQUESTHANDLER_H_ */
