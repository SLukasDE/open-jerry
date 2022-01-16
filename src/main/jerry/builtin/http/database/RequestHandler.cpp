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

#include <jerry/builtin/http/database/RequestHandler.h>
#include <jerry/Logger.h>

#include <esl/com/http/server/Response.h>
#include <esl/com/http/server/Request.h>
#include <esl/com/http/server/exception/StatusCode.h>
#include <esl/database/PreparedStatement.h>
#include <esl/io/output/Memory.h>
#include <esl/io/input/Closed.h>
#include <esl/utility/MIME.h>
#include <esl/Stacktrace.h>

#include <stdexcept>

namespace jerry {
namespace builtin {
namespace http {
namespace database {

namespace {
Logger logger("jerry::builtin::http::database::RequestHandler");

const std::string PAGE_200(
		"<!DOCTYPE html>\n"
		"<html>\n"
		"<head>\n"
		"<title>Database connection test</title>\n"
		"</head>\n"
		"<body>\n"
		"<h1>OK</h1>\n"
		"</body>\n"
		"</html>\n");

const std::string PAGE_500(
		"<!DOCTYPE html>\n"
		"<html>\n"
		"<head>\n"
		"<title>Database connection test</title>\n"
		"</head>\n"
		"<body>\n"
		"<h1>Failure</h1>\n"
		"</body>\n"
		"</html>\n");
} /* anonymous namespace */

std::unique_ptr<esl::com::http::server::requesthandler::Interface::RequestHandler> RequestHandler::createRequestHandler(const esl::module::Interface::Settings& settings) {
	return std::unique_ptr<esl::com::http::server::requesthandler::Interface::RequestHandler>(new RequestHandler(settings));
}

RequestHandler::RequestHandler(const esl::module::Interface::Settings& settings) {
	for(const auto& setting : settings) {
		if(setting.first == "connection-id") {
			if(!connectionId.empty()) {
				throw std::runtime_error("Multiple definition of attribute 'connection-id'");
			}
			connectionId = setting.second;
			if(connectionId.empty()) {
				throw std::runtime_error("Invalid value \"\" for attribute 'connection-id'");
			}
		}
		else if(setting.first == "sql") {
			if(!sql.empty()) {
				throw std::runtime_error("Multiple definition of attribute 'sql'");
			}
			sql = setting.second;
			if(sql.empty()) {
				throw std::runtime_error("Invalid value \"\" for attribute 'sql'");
			}
		}
		else {
			throw std::runtime_error("Unknown parameter key=\"" + setting.first + "\" with value=\"" + setting.second + "\"");
		}
	}

	if(connectionId.empty()) {
		throw std::runtime_error("Missing attribute 'connection-id'");
	}
	if(sql.empty()) {
		throw std::runtime_error("Missing attribute 'sql'");
	}
}

esl::io::Input RequestHandler::accept(esl::com::http::server::RequestContext& requestContext) const {
	if(connectionFactory == nullptr) {
        throw esl::com::http::server::exception::StatusCode(500, "Cannot find connection factory with id \"" + connectionId + "\"");
	}

	std::unique_ptr<esl::database::Connection> connection = connectionFactory->createConnection();
	if(!connection) {
		//throw esl::com::http::server::exception::StatusCode(503, "no connection available");
		esl::com::http::server::Response response(500, esl::utility::MIME(esl::utility::MIME::textHtml));
		esl::io::Output output = esl::io::output::Memory::create(PAGE_500.data(), PAGE_500.size());
		requestContext.getConnection().send(response, std::move(output));
		logger.debug << "Failure\n";

		return esl::io::input::Closed::create();
	}

	try {
	    esl::database::PreparedStatement preparedStatement = connection->prepare(sql);
	    preparedStatement.execute();
	}
	catch(...) {
		esl::com::http::server::Response response(500, esl::utility::MIME(esl::utility::MIME::textHtml));
		esl::io::Output output = esl::io::output::Memory::create(PAGE_500.data(), PAGE_500.size());
		requestContext.getConnection().send(response, std::move(output));
		logger.debug << "Failure\n";

		return esl::io::input::Closed::create();
	}


	esl::com::http::server::Response response(200, esl::utility::MIME(esl::utility::MIME::textHtml));
	esl::io::Output output = esl::io::output::Memory::create(PAGE_200.data(), PAGE_200.size());
	requestContext.getConnection().send(response, std::move(output));
	logger.debug << "OK\n";

	return esl::io::input::Closed::create();
}

void RequestHandler::initializeContext(esl::object::ObjectContext& objectContext) {
	connectionFactory = objectContext.findObject<esl::database::Interface::ConnectionFactory>(connectionId);
	if(connectionFactory == nullptr) {
		throw esl::addStacktrace(std::runtime_error("Cannot find connection factory with id \"" + connectionId + "\""));
	}
}

} /* namespace database */
} /* namespace http */
} /* namespace builtin */
} /* namespace jerry */
