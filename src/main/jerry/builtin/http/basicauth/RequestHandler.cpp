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

#include <jerry/builtin/http/basicauth/RequestHandler.h>
#include <jerry/builtin/http/basicauth/Settings.h>
#include <jerry/Logger.h>

#include <esl/com/http/server/Response.h>
#include <esl/utility/MIME.h>
#include <esl/io/output/Memory.h>

namespace jerry {
namespace builtin {
namespace http {
namespace basicauth {

namespace {
Logger logger("jerry::builtin::basicauth::RequestHandler");

const std::string PAGE_401(
		"<!DOCTYPE html>\n"
		"<html>\n"
		"<head>\n"
		"<title>401 - Unauthorized</title>\n"
		"</head>\n"
		"<body>\n"
		"<h1>401 - Unauthorized</h1>\n"
		"</body>\n"
		"</html>\n");
}

esl::io::Input RequestHandler::create(esl::com::http::server::RequestContext& requestContext) {
	const Settings* settings = requestContext.findObject<Settings>("");
	if(settings == nullptr) {
		return esl::io::Input();
	}

	if(requestContext.getRequest().getUsername() == settings->getUsername() &&
			requestContext.getRequest().getPassword() == settings->getPassword()) {
		return esl::io::Input();
	}

	return esl::io::Input(std::unique_ptr<esl::io::Writer>(new RequestHandler(requestContext, settings->getRealmId())));
}

RequestHandler::RequestHandler(esl::com::http::server::RequestContext& requestContext, const std::string& realmId)
{
	esl::com::http::server::Response response(401, esl::utility::MIME(esl::utility::MIME::textHtml), realmId);
	std::unique_ptr<esl::io::Producer> producer(new esl::io::output::Memory(PAGE_401.data(), PAGE_401.size()));
	requestContext.getConnection().send(response, esl::io::Output(std::move(producer)));
}

// if function is called with size=0, this signals that writing is done, so write will not be called anymore.
// -> this can be used for cleanup stuff.
std::size_t RequestHandler::write(const void* data, std::size_t size) {
	return esl::io::Writer::npos;
}

// returns consumable bytes to write.
// npos is returned if available size is unknown.
std::size_t RequestHandler::getSizeWritable() const {
	return esl::io::Writer::npos;
}


} /* namespace basicauth */
} /* namespace http */
} /* namespace builtin */
} /* namespace jerry */
