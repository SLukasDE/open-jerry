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

#include <jerry/builtin/procedure/authentication/jwt/Procedure.h>
#include <jerry/Logger.h>

#include <esl/utility/String.h>

#include "rapidjson/document.h"

#include <ctime>
#include <stdexcept>

namespace jerry {
namespace builtin {
namespace procedure {
namespace authentication {
namespace jwt {

namespace {
Logger logger("jerry::builtin::procedure::authentication::jwt::Procedure");
} /* anonymous namespace */

std::unique_ptr<esl::processing::procedure::Interface::Procedure> Procedure::createProcedure(const esl::module::Interface::Settings& settings) {
	return std::unique_ptr<esl::processing::procedure::Interface::Procedure>(new Procedure(settings));
}

Procedure::Procedure(const esl::module::Interface::Settings& settings) {
	for(const auto& setting : settings) {
		throw std::runtime_error("Unknown parameter key=\"" + setting.first + "\" with value=\"" + setting.second + "\"");
	}
}

void Procedure::procedureRun(esl::object::ObjectContext& objectContext) {
	Properties* authProperties = objectContext.findObject<Properties>("authenticated");
	if(!authProperties) {
		return;
	}

	auto iter = authProperties->get().find("type");
	if(iter == authProperties->get().end()) {
		return;
	}

	std::vector<std::string> typeSplit = esl::utility::String::split(iter->second, ',', true);
	bool jwtFound = false;
	for(const auto& type : typeSplit) {
		if(type == "jwt") {
			jwtFound = true;
			break;
		}
	}

	if(!jwtFound) {
		return;
	}

	std::string jwtPayloadStr = authProperties->get().at("jwt-payload");
	std::string jwtAudStr = authProperties->get().at("jwt-aud");
	rapidjson::Document document;
	document.Parse(jwtPayloadStr.c_str());

	if(!document.IsObject()) {
		return;
	}

	/* iss  Issuer           Der Aussteller des Tokens
	 * sub  Subject          Definiert für welches Subjekt die Claims gelten. Das sub-Feld definiert also für wen oder was die Claims getätigt werden.
	 * aud  Audience         Die Zieldomäne, für die das Token ausgestellt wurde.
	 * exp  Expiration Time  Das Ablaufdatum des Tokens in Unixzeit, also der Anzahl der Sekunden seit 1970-01-01T00:00:00Z.
	 * nbf  Not Before       Die Unixzeit, ab der das Token gültig ist.
	 * iat  Issued At        Die Unixzeit, zu der das Token ausgestellt wurde.
	 */
	if(document.HasMember("aud") && document["aud"].IsString()) {
		if(jwtAudStr != document["aud"].GetString()) {
			logger.warn << "Web-Token is issued for \"" << document["aud"].GetString() << "\" but used for \"" << jwtAudStr << "\".\n";
			return;
		}
	}
    std::time_t currentTime = std::time(nullptr);
	if(document.HasMember("exp") && document["exp"].IsUint64()) {
		if(currentTime > document["exp"].GetInt64()) {
			logger.warn << "Web-Token is expired. Token is valid to timestamp " << document["exp"].GetUint64() << " but current timestamp is " << currentTime << ".\n";
			return;
		}
	}
	if(document.HasMember("nbf") && document["nbf"].IsUint64()) {
		if(currentTime < document["nbf"].GetInt64()) {
			logger.warn << "Web-Token is still not valid. Token is valid from timestamp " << document["nbf"].GetUint64() << " but current timestamp is " << currentTime << ".\n";
			return;
		}
	}

	if(!document.HasMember("sub") || !document["sub"].IsString()) {
		authProperties->get()["identified"] = "";
	}
	else {
		authProperties->get()["identified"] = document["sub"].GetString();
	}

}

void Procedure::procedureCancel() {
}

} /* namespace jwt */
} /* namespace authentication */
} /* namespace procedure */
} /* namespace builtin */
} /* namespace jerry */
