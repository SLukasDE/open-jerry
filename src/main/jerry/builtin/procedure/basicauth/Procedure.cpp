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

#include <jerry/builtin/procedure/basicauth/Procedure.h>
#include <jerry/Logger.h>

#include <esl/object/Properties.h>

#include <stdexcept>

namespace jerry {
namespace builtin {
namespace procedure {
namespace basicauth {

namespace {
Logger logger("jerry::builtin::procedure::basicauth::Procedure");
} /* anonymous namespace */

std::unique_ptr<esl::processing::procedure::Interface::Procedure> Procedure::createProcedure(const esl::module::Interface::Settings& settings) {
	return std::unique_ptr<esl::processing::procedure::Interface::Procedure>(new Procedure(settings));
}

Procedure::Procedure(const esl::module::Interface::Settings& settings) {
	for(const auto& setting : settings) {
		if(setting.first == "credential") {
			if(setting.second.empty()) {
				throw std::runtime_error("Value \"\" of parameter 'credential' is invalid");

			}

			std::string username;
			bool hasType = false;
			Type type = plain;
			std::string password;

		    std::string::const_iterator currentIt = setting.second.begin();
		    std::string::const_iterator lastIt = currentIt;

		    for(;currentIt != setting.second.end(); ++currentIt) {
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
							throw std::runtime_error("Value \"" + setting.second + "\" of attribute 'credential' has unknown password type \"" + typeStr + "\"");
			        	}

			            ++currentIt;
			        	password = std::string(currentIt, setting.second.end());

		        		break;
		        	}
		        }
		    }

        	if(username.empty()) {
				throw std::runtime_error("Value \"" + setting.second + "\" of attribute 'credential' has empty username");
        	}
        	if(!hasType) {
				throw std::runtime_error("Value \"" + setting.second + "\" of attribute 'credential' has empty password type");
        	}
        	credentials.emplace_back(username, type, password);
		}
		else {
			throw std::runtime_error("Unknown parameter key=\"" + setting.first + "\" with value=\"" + setting.second + "\"");
		}
	}
}

void Procedure::procedureRun(esl::object::ObjectContext& objectContext) {
//	std::unique_ptr<esl::object::Interface::Object> properties(new esl::object::Properties(settings));
//	requestContext.getObjectContext().addObject("auth", std::move(properties));

	esl::object::Properties* authProperties = objectContext.findObject<esl::object::Properties>("auth");
	if(authProperties && authProperties->hasValue("type") && authProperties->getValue("type") == "basicauth") {
		std::string username = authProperties->getValue("username");
		std::string password = authProperties->getValue("password");

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

			authProperties->addValue("identified", username);
			//authProperties->addValue("ok", "true");
			break;
		}
	}
}

void Procedure::procedureCancel() {
}

} /* namespace basicauth */
} /* namespace procedure */
} /* namespace builtin */
} /* namespace jerry */
