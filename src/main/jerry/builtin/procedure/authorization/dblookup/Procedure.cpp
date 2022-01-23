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

#include <jerry/builtin/procedure/authorization/dblookup/Procedure.h>
#include <jerry/Logger.h>

#include <esl/database/PreparedStatement.h>
#include <esl/database/ResultSet.h>

#include <stdexcept>

namespace jerry {
namespace builtin {
namespace procedure {
namespace authorization {
namespace dblookup {

namespace {
Logger logger("jerry::builtin::procedure::dblookup::Procedure");
} /* anonymous namespace */

std::unique_ptr<esl::processing::procedure::Interface::Procedure> Procedure::createProcedure(const esl::module::Interface::Settings& settings) {
	return std::unique_ptr<esl::processing::procedure::Interface::Procedure>(new Procedure(settings));
}

Procedure::Procedure(const esl::module::Interface::Settings& settings) {
	bool hasAuthorizedObjectId = false;

	for(const auto& setting : settings) {
		if(setting.first == "authorized-object-id") {
			if(hasAuthorizedObjectId) {
				throw std::runtime_error("Multiple definition of attribute 'authorized-object-id'");
			}
			authorizedObjectId = setting.second;
			hasAuthorizedObjectId = true;
			if(authorizedObjectId.empty()) {
				throw std::runtime_error("Invalid value \"\" for attribute 'authorized-object-id'");
			}
		}
		else if(setting.first == "connection-id") {
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

void Procedure::procedureRun(esl::object::ObjectContext& objectContext) {
	if(connectionFactory == nullptr) {
		logger.warn << "No DB connection factory initialized.\n";
		return;
	}

	Properties* authProperties = objectContext.findObject<Properties>("authenticated");
	if(!authProperties) {
		return;
	}

	auto iter = authProperties->get().find("identified");
	if(iter == authProperties->get().end()) {
		return;
	}

	std::unique_ptr<esl::database::Connection> connection = connectionFactory->createConnection();
	if(!connection) {
		logger.warn << "Could not create DB connection\n";
		return;
	}

    esl::database::PreparedStatement preparedStatement = connection->prepare(sql);
    std::vector<std::string> fields;
	for(std::size_t i=0; i < preparedStatement.getResultColumns().size(); ++i) {
		fields.push_back(preparedStatement.getResultColumns()[i].getName());
	}

    esl::database::ResultSet resultSet = preparedStatement.execute(iter->second);
    if(!resultSet) {
    	return;
    }

    std::map<std::string, std::string> authorizationProperites;
	for(std::size_t i=0; i < fields.size(); ++i) {
		if(resultSet[i].isNull()) {
			authorizationProperites.insert(std::make_pair(fields[i], ""));
		}
		else {
			authorizationProperites.insert(std::make_pair(fields[i], resultSet[i].asString()));
		}
	}

	objectContext.addObject(authorizedObjectId, std::unique_ptr<esl::object::Interface::Object>(new Properties(std::move(authorizationProperites))));
}

void Procedure::initializeContext(esl::object::ObjectContext& objectContext) {
	connectionFactory = objectContext.findObject<esl::database::Interface::ConnectionFactory>(connectionId);
	if(connectionFactory == nullptr) {
		throw std::runtime_error("Cannot find connection factory with id \"" + connectionId + "\"");
	}
}

} /* namespace dblookup */
} /* namespace authorization */
} /* namespace procedure */
} /* namespace builtin */
} /* namespace jerry */
