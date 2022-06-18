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

#include <esl/com/http/client/Request.h>
#include <esl/com/http/client/Response.h>
#include <esl/io/Input.h>
#include <esl/io/input/String.h>
#include <esl/io/Output.h>
#include <esl/utility/HttpMethod.h>
#include <esl/utility/MIME.h>
#include <esl/utility/String.h>

#include "rapidjson/document.h"

#include <gnutls/gnutls.h>

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

std::unique_ptr<esl::processing::procedure::Interface::Procedure> Procedure::create(const std::vector<std::pair<std::string, std::string>>& settings) {
	return std::unique_ptr<esl::processing::procedure::Interface::Procedure>(new Procedure(settings));
}

Procedure::Procedure(const std::vector<std::pair<std::string, std::string>>& settings) {
	for(const auto& setting : settings) {
		if(setting.first == "drop-field") {
			if(setting.second.empty()) {
				throw std::runtime_error("Invalid value \"\" for attribute 'drop-field'");
			}
			dropFields.insert(setting.second);
		}
		else if(setting.first == "override-field") {
			std::size_t pos = setting.second.find(':');
			if(pos==std::string::npos) {
				throw std::runtime_error("Invalid value \"" + setting.second + "\" for attribute 'override-field'. Value should look like <field>:<value>");
			}

			overrideFields.insert(std::make_pair(setting.second.substr(0, pos), setting.second.substr(pos+1)));
		}
		else if(setting.first == "jwks-client-id") {
			if(setting.second.empty()) {
				throw std::runtime_error("Invalid value \"\" for attribute 'jwks-client-id'");
			}
			jwksConnectionFactoryIds.insert(setting.second);
		}
		else {
			throw std::runtime_error("Unknown parameter key=\"" + setting.first + "\" with value=\"" + setting.second + "\"");
		}
	}
}

void Procedure::initializeContext(esl::object::Context& objectContext) {
	for(const auto jwksConnectionFactoryId : jwksConnectionFactoryIds) {
		esl::com::http::client::Interface::ConnectionFactory* connectionFactory = objectContext.findObject<esl::com::http::client::Interface::ConnectionFactory>(jwksConnectionFactoryId);
		if(!connectionFactory) {
			throw std::runtime_error("HTTP client with id \"" + jwksConnectionFactoryId + "\" not found");
		}
		jwksConnectionFactories.insert(std::make_pair(jwksConnectionFactoryId, std::ref(*connectionFactory)));
	}
}

void Procedure::procedureRun(esl::object::Context& objectContext) {
	Properties* authProperties = objectContext.findObject<Properties>("authenticated");
	if(!authProperties) {
		return;
	}

	auto authIter = authProperties->get().find("type");
	if(authIter == authProperties->get().end()) {
		return;
	}

	std::vector<std::string> typeSplit = esl::utility::String::split(authIter->second, ',', true);
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

	/* **************** *
	 * Verification JWT *
	 * **************** */

    std::time_t currentTime = std::time(nullptr);

	/* iss  Issuer           Der Aussteller des Tokens
	 * sub  Subject          Definiert für welches Subjekt die Claims gelten. Das sub-Feld definiert also für wen oder was die Claims getätigt werden.
	 * aud  Audience         Die Zieldomäne, für die das Token ausgestellt wurde.
	 * exp  Expiration Time  Das Ablaufdatum des Tokens in Unixzeit, also der Anzahl der Sekunden seit 1970-01-01T00:00:00Z.
	 * nbf  Not Before       Die Unixzeit, ab der das Token gültig ist.
	 * iat  Issued At        Die Unixzeit, zu der das Token ausgestellt wurde.
	 */

	std::string jwtPayloadStr = authProperties->get().at("jwt-payload");
	rapidjson::Document document;
	document.Parse(jwtPayloadStr.c_str());

	if(!document.IsObject()) {
		logger.warn << "JWT payload content is not a JSON object.\n";
		return;
	}

	/* ************ *
	 * verify 'aud' *
	 * ************ */
	std::string aud;
	if(dropFields.count("aud") != 0) {
		aud = authProperties->get().at("jwt-aud");
	}
	else if(overrideFields.count("aud") != 0) {
		aud = overrideFields.at("aud");
	}
	else if(document.HasMember("aud") && document["aud"].IsString()) {
		aud = document["aud"].GetString();
	}

	if(aud != authProperties->get().at("jwt-aud")) {
		logger.warn << "Web-Token is issued for \"" << overrideFields.at("aud") << "\" but used for \"" << authProperties->get().at("jwt-aud") << "\".\n";
		return;
	}

	/* ************ *
	 * verify 'exp' *
	 * ************ */
    std::time_t exp = 0;
	if(dropFields.count("exp") != 0) {
		exp = currentTime;
	}
	else if(overrideFields.count("exp") != 0) {
		exp = std::stol(overrideFields.at("exp"));
	}
	else if(document.HasMember("exp") && document["exp"].IsUint64()) {
		exp = document["exp"].GetInt64();
	}

	if(currentTime > exp) {
		logger.warn << "Web-Token is expired. Token is valid to timestamp " << exp << " but current timestamp is " << currentTime << ".\n";
		return;
	}

	/* ************ *
	 * verify 'nbf' *
	 * ************ */
    std::time_t nbf = 0;
	if(dropFields.count("nbf") != 0) {
		nbf = currentTime;
	}
	else if(overrideFields.count("nbf") != 0) {
		exp = std::stol(overrideFields.at("nbf"));
	}
	else if(document.HasMember("nbf") && document["nbf"].IsUint64()) {
		exp = document["nbf"].GetInt64();
	}

	if(currentTime < nbf) {
		logger.warn << "Web-Token is still not valid. Token is valid from timestamp " << nbf << " but current timestamp is " << currentTime << ".\n";
		return;
	}

	/* **************** *
	 * verify signature *
	 * **************** */
	if(authProperties->get().count("jwt-signature") != 0) {
		std::string jwtHeaderStr = authProperties->get().at("jwt-header");
		rapidjson::Document document;
		document.Parse(jwtHeaderStr.c_str());

		if(!document.IsObject()) {
			logger.warn << "JWT header content is not a JSON object.\n";
			return;
		}

		std::string kid;
		if(overrideFields.count("kid") != 0) {
			kid = overrideFields.at("kid");
		}
		else if(document.HasMember("kid") && document["kid"].IsString()) {
			kid = document["kid"].GetString();
		}
		else {
			logger.warn << "Field \"kid\" is missing in JWT header.\n";
		}

		std::string alg;
		if(overrideFields.count("alg") != 0) {
			alg = overrideFields.at("alg");
		}
		else if(document.HasMember("alg") && document["alg"].IsString()) {
			alg = document["alg"].GetString();
		}
		else {
			logger.warn << "Field \"alg\" is missing in JWT header.\n";
		}

		std::pair<gtx::PublicKey*, std::string> publicKey = getPublicKeyById(kid);
		if(publicKey.first) {
			std::string data = authProperties->get().at("jwt-data");
			std::string signature = authProperties->get().at("jwt-signature");

			if(alg.empty()) {
				alg = publicKey.second;
			}
			logger.trace << "JWT kid : \"" << kid << "\"\n";
			logger.trace << "JWT alg : \"" << alg << "\"\n";
			logger.trace << "JWT data: \"" << data << "\" (" << data.size() << " bytes)\n";
			logger.trace << "JWT sign: \"...\" (" << signature.size() << " bytes)\n";

			if(publicKey.first->verifySignature(data, signature, alg) == false) {
				logger.warn << "JWT verification failed because signature is invalid.\n";
				return;
			}
			logger.warn << "JWT verification SUCCESSFUL.\n";
		}
		else if(kid.empty()) {
			return;
		}
		else {
			logger.warn << "JWT verification failed because public key is not available.\n";
			return;
		}
	}

	/* ****************** *
	 * fetch 'identified' *
	 * ****************** */
	if(dropFields.count("sub") != 0) {
		authProperties->get()["identified"] = "";
	}
	else if(overrideFields.count("sub") != 0) {
		authProperties->get()["identified"] = overrideFields.at("sub");
	}
	else if(document.HasMember("sub") && document["sub"].IsString()) {
		authProperties->get()["identified"] = document["sub"].GetString();
	}
	else {
		authProperties->get()["identified"] = "";
	}
}

void Procedure::procedureCancel() {
}

std::pair<gtx::PublicKey*, std::string> Procedure::getPublicKeyById(const std::string& kid) {
	if(publicKeyById.count(kid) != 0) {
		return std::make_pair(publicKeyById.at(kid).first.get(), publicKeyById.at(kid).second);
	}

	for(auto& jwksConnectionFactory : jwksConnectionFactories) {
		auto connection = jwksConnectionFactory.second.get().createConnection();
		if(!connection) {
			logger.warn << "Could not get an connection object for JWKS server with id \"" << jwksConnectionFactory.first << "\".\n";
			continue;
		}
		esl::com::http::client::Request request("", esl::utility::HttpMethod::Type::httpGet, esl::utility::MIME());
		esl::io::input::String inputString;

		esl::com::http::client::Response response = connection->send(request, esl::io::Output(), esl::io::Input(inputString));

		if(response.getStatusCode() < 200 || response.getStatusCode() > 299) {
			logger.warn << "JWKS server response with HTTP status code " << response.getStatusCode() << ".\n";
			continue;
		}

		rapidjson::Document document;
		document.Parse(inputString.getString().c_str());

		logger.info << "HTTP response:\n";
		logger.info << "- content type: " << response.getContentType().toString() << "\n";

		if(!document.IsObject()) {
			logger.warn << "JWKS content is not a JSON object.\n";
			continue;
		}

		if(!document.HasMember("keys") || !document["keys"].IsArray()) {
			logger.warn << "JWKS object has no \"keys\" array.\n";
		}

		auto jsonArray = document["keys"].GetArray();
		for(rapidjson::Value::ConstValueIterator iter = jsonArray.Begin(); iter != jsonArray.End(); ++iter) {
			if(!iter->IsObject()) {
				logger.warn << "JWK object has no \"keys\" array.\n";
				continue;
			}

			std::string kid;
			if(iter->HasMember("kid") && (*iter)["kid"].IsString()) {
				kid = (*iter)["kid"].GetString();
			}

			if(!iter->HasMember("kty") || !(*iter)["kty"].IsString()) {
				logger.warn << "JWK object has no string member \"kty\".\n";
				continue;
			}
			std::string kty = (*iter)["kty"].GetString();

			if(kty == "RSA") {
				std::string use = "sig";
				if(iter->HasMember("use") && (*iter)["use"].IsString()) {
					use = (*iter)["use"].GetString();
				}
				if(use != "sig") {
					continue;
				}

				if(!iter->HasMember("n") || !(*iter)["n"].IsString()) {
					logger.warn << "JWK object has no string member \"n\".\n";
					continue;
				}
				std::string modulus = esl::utility::String::fromBase64((*iter)["n"].GetString());

				if(!iter->HasMember("e") || !(*iter)["e"].IsString()) {
					logger.warn << "JWK object has no string member \"e\".\n";
					continue;
				}
				std::string exponent = esl::utility::String::fromBase64((*iter)["e"].GetString());

				std::string alg = "RS256";
				if(iter->HasMember("alg") && (*iter)["alg"].IsString()) {
					alg = (*iter)["alg"].GetString();
				}

				logger.info << "Store public key for KID \"" << kid << "\" with algorithm \"" << alg << "\"\n";
				publicKeyById.insert(std::make_pair(kid, std::make_pair(gtx::PublicKey::createRSA(exponent, modulus), alg)));
			}
			else if(kty == "EC") {
				std::string use = "sig";
				if(iter->HasMember("use") && (*iter)["use"].IsString()) {
					use = (*iter)["use"].GetString();
				}
				if(use != "sig") {
					continue;
				}

				if(!iter->HasMember("x") || !(*iter)["x"].IsString()) {
					logger.warn << "JWK object has no string member \"x\".\n";
					continue;
				}
				std::string coordinateX = (*iter)["x"].GetString();

				if(!iter->HasMember("y") || !(*iter)["y"].IsString()) {
					logger.warn << "JWK object has no string member \"y\".\n";
					continue;
				}
				std::string coordinateY = (*iter)["y"].GetString();

				std::string crv = "P-256";
				if(iter->HasMember("crv") && (*iter)["crv"].IsString()) {
					crv = (*iter)["crv"].GetString();
				}

				gnutls_ecc_curve_t curveType = GNUTLS_ECC_CURVE_INVALID;
				if(crv == "P-192") {
					curveType = GNUTLS_ECC_CURVE_SECP192R1;
				}
				else if(crv == "P-224") {
					curveType = GNUTLS_ECC_CURVE_SECP224R1;
				}
				else if(crv == "P-256") {
					curveType = GNUTLS_ECC_CURVE_SECP256R1;
				}
				else if(crv == "P-384") {
					curveType = GNUTLS_ECC_CURVE_SECP384R1;
				}
				else if(crv == "P-521") {
					curveType = GNUTLS_ECC_CURVE_SECP521R1;
				}
				else {
					logger.warn << "JWK object has an unknown EC curve type \"" << crv << "\".\n";
					continue;
				}

				std::string alg = "ES256";
				if(iter->HasMember("alg") && (*iter)["alg"].IsString()) {
					alg = (*iter)["alg"].GetString();
				}

				publicKeyById.insert(std::make_pair(kid, std::make_pair(gtx::PublicKey::createEC(curveType, coordinateX, coordinateY), alg)));
			}
			else {
				logger.warn << "Value \"" << kty << "\" of JWK member \"kty\" is not supported. Supported values are \"RSA\" and \"EC\".\n";
				continue;
			}
		}

		if(publicKeyById.count(kid) != 0) {
			return std::make_pair(publicKeyById.at(kid).first.get(), publicKeyById.at(kid).second);
		}
	}

	return std::make_pair(nullptr, "");
}

} /* namespace jwt */
} /* namespace authentication */
} /* namespace procedure */
} /* namespace builtin */
} /* namespace jerry */
