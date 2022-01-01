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

#include <jerry/builtin/http/proxy/RequestHandler.h>
#include <jerry/Logger.h>

#include <esl/com/http/client/Response.h>
#include <esl/com/http/client/Request.h>
#include <esl/com/http/server/Response.h>
#include <esl/com/http/server/Request.h>
#include <esl/com/http/server/exception/StatusCode.h>
//#include <esl/com/http/client/Connection.h>
#include <esl/io/input/String.h>
#include <esl/io/input/Closed.h>
#include <esl/io/output/String.h>
#include <esl/utility/MIME.h>
#include <esl/Stacktrace.h>

#include <stdexcept>

namespace jerry {
namespace builtin {
namespace http {
namespace proxy {

namespace {
Logger logger("jerry::builtin::http::proxy::RequestHandler");
} /* anonymous namespace */

std::unique_ptr<esl::com::http::server::requesthandler::Interface::RequestHandler> RequestHandler::createRequestHandler(const esl::object::Interface::Settings& settings) {
	return std::unique_ptr<esl::com::http::server::requesthandler::Interface::RequestHandler>(new RequestHandler(settings));
}

RequestHandler::RequestHandler(const esl::object::Interface::Settings& settings) {
	for(const auto& setting : settings) {
		if(setting.first == "http-client-id") {
			httpClientId = setting.second;
		}
		else {
			throw esl::addStacktrace(std::runtime_error("Unknown parameter key=\"" + setting.first + "\" with value=\"" + setting.second + "\""));
		}
	}
}

esl::io::Input RequestHandler::accept(esl::com::http::server::RequestContext& requestContext, esl::object::Interface::ObjectContext& objectContext) const {
	if(connectionFactory == nullptr) {
        throw esl::com::http::server::exception::StatusCode(500, "Initialization failed, http-client not found with id \"" + httpClientId + "\"");
	}

	std::unique_ptr<esl::com::http::client::Interface::Connection> connection = connectionFactory->createConnection();
	if(!connection) {
		throw esl::com::http::server::exception::StatusCode(503, "no client connection available");
	}

	esl::com::http::client::Request clientRequest(requestContext.getPath(), esl::utility::HttpMethod::httpGet, esl::utility::MIME());
	esl::io::input::String clientString;
	esl::com::http::client::Response clientResponse = connection->send(clientRequest, esl::io::Output(), esl::io::Input(clientString));

	esl::com::http::server::Response serverResponse(clientResponse.getStatusCode(), clientResponse.getContentType());
	esl::io::Output output = esl::io::output::String::create(clientString.getString());
	requestContext.getConnection().send(serverResponse, std::move(output));

	return esl::io::input::Closed::create();
}

void RequestHandler::initializeContext(esl::object::Interface::ObjectContext& objectContext) {
	connectionFactory = objectContext.findObject<esl::com::http::client::Interface::ConnectionFactory>(httpClientId);
	if(connectionFactory == nullptr) {
		throw esl::addStacktrace(std::runtime_error("Cannot find http-client with id \"" + httpClientId + "\""));
	}
}

} /* namespace proxy */
} /* namespace http */
} /* namespace builtin */
} /* namespace jerry */
