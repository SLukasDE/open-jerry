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

#include <jerry/builtin/procedure/authorization/jwt/Procedure.h>
#include <jerry/Logger.h>

#include <esl/utility/String.h>

#include "rapidjson/document.h"

#include <ctime>
#include <stdexcept>

namespace jerry {
namespace builtin {
namespace procedure {
namespace authorization {
namespace jwt {

namespace {
Logger logger("jerry::builtin::procedure::authorization::jwt::Procedure");
} /* anonymous namespace */

std::unique_ptr<esl::processing::procedure::Interface::Procedure> Procedure::create(const std::vector<std::pair<std::string, std::string>>& settings) {
	return std::unique_ptr<esl::processing::procedure::Interface::Procedure>(new Procedure(settings));
}

Procedure::Procedure(const std::vector<std::pair<std::string, std::string>>& settings) {
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
		else {
			throw std::runtime_error("Unknown parameter key=\"" + setting.first + "\" with value=\"" + setting.second + "\"");
		}
	}
}

void Procedure::procedureRun(esl::object::ObjectContext& objectContext) {
	Properties* authProperties = objectContext.findObject<Properties>("authenticated");
	if(!authProperties) {
		return;
	}

	auto iter = authProperties->get().find("jwt-payload");
	if(iter == authProperties->get().end()) {
		return;
	}

	std::map<std::string, std::string> authorizedProperties;

	rapidjson::Document document;
	document.Parse(iter->second.c_str());
	if(document.IsObject()) {
		for(rapidjson::Value::ConstMemberIterator jsonIter = document.MemberBegin(); jsonIter != document.MemberEnd(); ++jsonIter) {
			const char* key = jsonIter->name.GetString();
			if(document[key].IsInt64()) {
				authorizedProperties[key] = std::to_string(document[key].GetInt64());
			}
			else if(document[key].IsString()) {
				authorizedProperties[key] = document[key].GetString();
			}
		}
	}
	objectContext.addObject(authorizedObjectId, std::unique_ptr<esl::object::Interface::Object>(new Properties(std::move(authorizedProperties))));
}

void Procedure::procedureCancel() {
}

} /* namespace jwt */
} /* namespace authorization */
} /* namespace procedure */
} /* namespace builtin */
} /* namespace jerry */
