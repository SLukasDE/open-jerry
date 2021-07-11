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

#include <jerry/builtin/http/database/Settings.h>

#include <esl/Stacktrace.h>
#include <esl/com/http/server/exception/StatusCode.h>
#include <esl/database/PreparedStatement.h>

#include <stdexcept>

namespace jerry {
namespace builtin {
namespace http {
namespace database {

Settings::Settings(const esl::object::Interface::Settings& settings) {
	for(const auto& setting : settings) {
		if(setting.first == "connectionId") {
			connectionId = setting.second;
		}
		else if(setting.first == "SQL") {
			sql = setting.second;
		}
		else {
			throw esl::addStacktrace(std::runtime_error("Unknown parameter key=\"" + setting.first + "\" with value=\"" + setting.second + "\""));
		}
	}
}

void Settings::initializeContext(esl::object::ObjectContext& objectContext) {
	connectionFactory = objectContext.findObject<esl::database::Interface::ConnectionFactory>(getConnectionId());
	if(connectionFactory == nullptr) {
		throw esl::addStacktrace(std::runtime_error("Cannot find connection factory with id \"" + getConnectionId() + "\""));
	}
}

bool Settings::check() const {
	if(connectionFactory == nullptr) {
        throw esl::com::http::server::exception::StatusCode(500, "Cannot find connection factory with id \"" + connectionId + "\"");
	}

	std::unique_ptr<esl::database::Connection> connection = createConnection();
	if(!connection) {
		//throw esl::com::http::server::exception::StatusCode(503, "no connection available");
		return false;
	}

	try {
	    esl::database::PreparedStatement preparedStatement = connection->prepare(getSQL());
	    preparedStatement.execute();
	}
	catch(...) {
		return false;
	}

	return true;
}

const std::string& Settings::getConnectionId() const {
	return connectionId;
}

const std::string& Settings::getSQL() const {
	return sql;
}

std::unique_ptr<esl::database::Connection> Settings::createConnection() const {
	if(connectionFactory == nullptr) {
        throw esl::com::http::server::exception::StatusCode(500, "Cannot find connection factory with id \"" + connectionId + "\"");
	}

	return connectionFactory->createConnection();
}

} /* namespace database */
} /* namespace http */
} /* namespace builtin */
} /* namespace jerry */
