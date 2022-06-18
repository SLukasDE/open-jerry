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

#include <jerry/builtin/procedure/authentication/basic/stable/Procedure.h>
#include <jerry/Logger.h>

#include <esl/utility/String.h>

#include <stdexcept>

namespace jerry {
namespace builtin {
namespace procedure {
namespace authentication {
namespace basic {
namespace stable {

namespace {
Logger logger("jerry::builtin::procedure::authentication::basic::stable::Procedure");
} /* anonymous namespace */

std::unique_ptr<esl::processing::procedure::Interface::Procedure> Procedure::create(const std::vector<std::pair<std::string, std::string>>& settings) {
	return std::unique_ptr<esl::processing::procedure::Interface::Procedure>(new Procedure(settings));
}

Procedure::Procedure(const std::vector<std::pair<std::string, std::string>>& settings) {
	for(const auto& setting : settings) {
		if(setting.first == "credential") {
			if(setting.second.empty()) {
				throw std::runtime_error("Value \"\" of parameter 'credential' is invalid");
			}
        	credentials.push_back(parseCredential(setting.second));
		}
		else {
			throw std::runtime_error("Unknown parameter key=\"" + setting.first + "\" with value=\"" + setting.second + "\"");
		}
	}
}

void Procedure::procedureRun(esl::object::Context& objectContext) {
	Properties* authProperties = objectContext.findObject<Properties>("authenticated");
	if(!authProperties) {
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

	for(const auto& credential : credentials) {
		if(std::get<0>(credential) != username) {
			continue;
		}
		if(std::get<1>(credential) == plain) {
			if(std::get<2>(credential) != password) {
				continue;
			}
		}
		else {
			continue;
		}

		authProperties->get()["identified"] = username;
		return;
	}
}

void Procedure::procedureCancel() {
}

std::tuple<std::string, Procedure::Type, std::string> Procedure::parseCredential(const std::string& credential) {
	if(credential.empty()) {
		throw std::runtime_error("Credential \"\" is invalid");
	}

	std::string username;
	bool hasType = false;
	Type type = plain;
	std::string password;

    std::string::const_iterator currentIt = credential.begin();
    std::string::const_iterator lastIt = currentIt;

    for(;currentIt != credential.end(); ++currentIt) {
        if(*currentIt == ':') {
        	if(username.empty()) {
	        	username = std::string(lastIt, currentIt);
	        	if(username.empty()) {
	        		break;
	        	}
	            lastIt = currentIt;
	            ++lastIt;
        	}
        	else {
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
    }

	if(username.empty()) {
		throw std::runtime_error("Credential \"" + credential + "\" has empty username");
	}
	if(!hasType) {
		throw std::runtime_error("Credential \"" + credential + "\" has empty password type");
	}

	return std::tuple<std::string, Procedure::Type, std::string>(username, type, password);
}

} /* namespace stable */
} /* namespace basic */
} /* namespace authentication */
} /* namespace procedure */
} /* namespace builtin */
} /* namespace jerry */
