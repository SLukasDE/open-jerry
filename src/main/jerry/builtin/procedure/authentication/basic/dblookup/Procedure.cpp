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

#include <jerry/builtin/procedure/authentication/basic/dblookup/Procedure.h>
#include <jerry/Logger.h>

#include <esl/utility/String.h>

#include <stdexcept>

namespace jerry {
namespace builtin {
namespace procedure {
namespace authentication {
namespace basic {
namespace dblookup {

namespace {
Logger logger("jerry::builtin::procedure::authentication::basic::dblookup2::Procedure");
} /* anonymous namespace */

std::unique_ptr<esl::processing::Procedure> Procedure::create(const std::vector<std::pair<std::string, std::string>>& settings) {
	return std::unique_ptr<esl::processing::Procedure>(new Procedure(settings));
}

Procedure::Procedure(const std::vector<std::pair<std::string, std::string>>& settings) {
	bool hasLifetimeRenew = false;

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
		else if(setting.first == "lifetime-renew") {
			if(hasLifetimeRenew) {
				throw std::runtime_error("Multiple definition of attribute 'lifetime-renew'");
			}
			if(setting.second == "true") {
				lifetimeRenew = true;
			}
			else if(setting.second == "false") {
				lifetimeRenew = false;
			}
			else {
				throw std::runtime_error("Unknown value \"" + setting.second + "\" for parameter key=\"lifetime-renew\". Possible values are \"true\" or \"false\".");
			}
			hasLifetimeRenew = true;
		}
		else if(setting.first == "lifetime-ms") {
			if(lifetimeMs != std::chrono::milliseconds(0)) {
				throw std::runtime_error("Multiple definition of attribute 'lifetime-ms'");
			}
			try {
				lifetimeMs = std::chrono::milliseconds(std::stoul(setting.second));
			}
			catch(const std::exception& e) {
				throw std::runtime_error("Value \"" + setting.second + "\" of parameter 'lifetime-ms' is invalid. " + e.what());
			}
			catch(...) {
				throw std::runtime_error("Value \"" + setting.second + "\" of parameter 'lifetime-ms' is invalid.");
			}
			if(lifetimeMs == std::chrono::milliseconds(0)) {
				throw std::runtime_error("Value \"0\" of parameter 'lifetime-ms' is invalid");
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

	if(lifetimeMs == std::chrono::milliseconds(0)) {
		throw std::runtime_error("Parameter 'lifetime-ms' is missing");
	}
}

void Procedure::procedureRun(esl::object::Context& objectContext) {
	Properties* authProperties = objectContext.findObject<Properties>("authenticated");
	if(!authProperties) {
		return;
	}

	if(authProperties->get().count("identified") > 0) {
		return;
	}

	auto iter = authProperties->get().find("type");
	if(iter == authProperties->get().end()) {
		return;
	}

	std::vector<std::string> typeSplit = esl::utility::String::split(iter->second, ',', true);
	bool basicauthFound = false;
	for(const auto& type : typeSplit) {
		if(type == "basicauth") {
			basicauthFound = true;
			break;
		}
	}

	if(!basicauthFound) {
		return;
	}

	std::string username = authProperties->get().at("basicauth-username");
	std::string password = authProperties->get().at("basicauth-password");

	/* lookup for an authorization object in our session pool */
	auto object = sessionPool->get(username, username);

	/* if no object has been created then sessionPool-get(...) should have returned the existing object */
	if(!object) {
		logger.warn << "No credentials found\n";
		return;
	}

	/* the object should be give back into the session pool, so we have to clone it to put it into our object context */
	Credential* credential = object.get();
	if(credential) {
		if(std::get<0>(*credential) == plain) {
			if(std::get<1>(*credential) != password) {
				return;
			}
		}
		else {
			return;
		}

		authProperties->get()["identified"] = username;
		return;
	}
	else {
		logger.warn << "Credential is null\n.";
	}
}

void Procedure::procedureCancel() {
}

void Procedure::initializeContext(esl::object::Context& objectContext) {
	connectionFactory = objectContext.findObject<esl::database::ConnectionFactory>(connectionId);
	if(connectionFactory == nullptr) {
		throw std::runtime_error("Cannot find connection factory with id \"" + connectionId + "\"");
	}

	sessionPool.reset(new SessionPool([this](const std::string& username) {
		return loadCredentialsDynamic(username);
	}, 10, lifetimeMs, lifetimeRenew, false));
}

Procedure::Credential Procedure::parseCredential(const std::string& credential) {
	if(credential.empty()) {
		throw std::runtime_error("Credential \"\" is invalid");
	}

	bool hasType = false;
	Type type = plain;
	std::string password;

    std::string::const_iterator currentIt = credential.begin();
    std::string::const_iterator lastIt = currentIt;

    for(;currentIt != credential.end(); ++currentIt) {
        if(*currentIt == ':') {
    		hasType = true;
        	std::string typeStr = std::string(lastIt, currentIt);
        	if(typeStr == "plain") {
        		type = plain;
        	}
        	else {
				throw std::runtime_error("Credential \"" + credential + "\" has unknown password type \"" + typeStr + "\"");
        	}

            ++currentIt;
        	password = std::string(currentIt, credential.end());

    		break;
        }
    }

	if(!hasType) {
		throw std::runtime_error("Credential \"" + credential + "\" has empty password type");
	}

	return Credential(type, password);
}

std::unique_ptr<Procedure::Credential> Procedure::loadCredentialsDynamic(const std::string& username) {
	if(connectionFactory == nullptr) {
		logger.warn << "No DB connection factory initialized.\n";
		return nullptr;
	}

	std::unique_ptr<esl::database::Connection> connection = connectionFactory->createConnection();
	if(!connection) {
		logger.warn << "Could not create DB connection\n";
		return nullptr;
	}

    esl::database::PreparedStatement preparedStatement = connection->prepare(sql);
	if(preparedStatement.getResultColumns().size() != 1) {
		logger.warn << "SQL result set has " << preparedStatement.getResultColumns().size() << " columns, but should have 1 column.\n";
		return nullptr;
	}

    esl::database::ResultSet resultSet = preparedStatement.execute(username);
    if(!resultSet) {
    	logger.warn << "User \"" << username << "\" not found in database.\n";
    	return nullptr;
    }

	if(resultSet[0].isNull()) {
    	logger.warn << "No password defined for user \"" << username << "\" in database.\n";
    	return nullptr;
	}

	return std::unique_ptr<Credential>(new Credential(parseCredential(resultSet[0].asString())));
}

} /* namespace dblookup */
} /* namespace basic */
} /* namespace authentication */
} /* namespace procedure */
} /* namespace builtin */
} /* namespace jerry */
