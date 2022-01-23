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

#include <jerry/builtin/http/authentication/RequestHandler.h>
#include <jerry/Logger.h>

#include <esl/com/http/server/Request.h>
#include <esl/com/http/server/exception/StatusCode.h>
#include <esl/io/input/Closed.h>
#include <esl/io/output/Memory.h>
#include <esl/object/Interface.h>
#include <esl/utility/String.h>
#include <esl/utility/MIME.h>

#include "rapidjson/document.h"

#include <stdexcept>

namespace jerry {
namespace builtin {
namespace http {
namespace authentication {

namespace {
Logger logger("jerry::builtin::http::authentication::RequestHandler");

const std::string PAGE_401(
		"<!DOCTYPE html>\n"
		"<html>\n"
		"<head>\n"
		"<title>401 - Unauthorized</title>\n"
		"</head>\n"
		"<body>\n"
		"<h1>401 - Unauthorized</h1>\n"
		"</body>\n"
		"</html>\n");
} /* anonymous namespace */

std::unique_ptr<esl::com::http::server::requesthandler::Interface::RequestHandler> RequestHandler::createRequestHandler(const esl::module::Interface::Settings& settings) {
	return std::unique_ptr<esl::com::http::server::requesthandler::Interface::RequestHandler>(new RequestHandler(settings));
}

RequestHandler::RequestHandler(const esl::module::Interface::Settings& settings) {
	bool hasBehavior = false;

	for(const auto& setting : settings) {
		if(setting.first == "behavior") {
			if(hasBehavior) {
				throw std::runtime_error("Multiple definition of attribute 'behavior'");
			}

			hasBehavior = true;

			if(setting.second == "reply") {
				behavior = reply;
			}
			else if(setting.second == "proceed") {
				behavior = proceed;
			}
			else if(setting.second == "disconnect") {
				behavior = disconnect;
			}
			else {
				throw std::runtime_error("Invalid value \"" + setting.second + "\" for attribute 'behavior'");
			}
		}
		else if(setting.first == "allow") {
			if(setting.second == "basic") {
				allows.insert(basic);
			}
			else if(setting.second == "bearer") {
				allows.insert(bearer);
			}
			else if(setting.second == "cookie") {
				allows.insert(cookie);
			}
			else {
				throw std::runtime_error("Invalid value \"" + setting.second + "\" for attribute 'allow'");
			}
		}
		else if(setting.first == "authentication-procedure-id") {
			if(authenticationProceduresId.count(setting.second) == 0) {
				if(setting.second.empty()) {
					throw std::runtime_error("Invalid value \"\" for attribute 'authentication-procedure-id'");
				}
				authenticationProceduresId.insert(setting.second);
			}
		}
		else {
			throw std::runtime_error("Unknown parameter key=\"" + setting.first + "\" with value=\"" + setting.second + "\"");
		}
	}

	if(authenticationProceduresId.empty()) {
		logger.warn << "No procedures defined to verify authentication.\n";
	}
}

esl::io::Input RequestHandler::accept(esl::com::http::server::RequestContext& requestContext) const {
	processRequest(requestContext);

	return processResponse(requestContext);
}

void RequestHandler::initializeContext(esl::object::ObjectContext& objectContext) {
	for(const auto& authenticationProcedureId : authenticationProceduresId) {
		esl::processing::procedure::Interface::Procedure* authenticationProcedure = objectContext.findObject<esl::processing::procedure::Interface::Procedure>(authenticationProcedureId);
		if(authenticationProcedure == nullptr) {
			throw std::runtime_error("Cannot find authentication-procedure with id \"" + authenticationProcedureId + "\"");
		}
		authenticationProcedures.push_back(authenticationProcedure);
	}
}

void RequestHandler::processRequest(esl::com::http::server::RequestContext& requestContext) const {
	Settings settings;
	std::set<std::string> authDataTypes;

	if(allows.count(basic) > 0) {
		/* ******************************************************************** *
		 * Add Basic-Auth parameters to authDataTypes and settings if available *
		 * ******************************************************************** */
		processRequestBasicAuth(authDataTypes, settings, requestContext);
	}

	if(allows.count(bearer) > 0) {
		/* ************************************************************************ *
		 * Add JSON Web Token parameters to authDataTypes and settings if available *
		 * ************************************************************************ */
		processRequestJWT(authDataTypes, settings, requestContext);
	}

	if(authDataTypes.empty()) {
		logger.warn << "no authorization data found.\n";
		return;
	}

	std::string typeStr;
	for(const auto& authDataType : authDataTypes) {
		if(!typeStr.empty()) {
			typeStr += ",";
		}
		typeStr += authDataType;
	}
	settings["type"] = std::move(typeStr);

	esl::object::ObjectContext& objectContext = requestContext.getObjectContext();
	std::unique_ptr<esl::object::Interface::Object> properties(new Properties(settings));
	objectContext.addObject("authenticated", std::move(properties));

	Properties* authProperties = objectContext.findObject<Properties>("authenticated");
	if(authProperties == nullptr) {
		return;
	}

	if(authenticationProcedures.empty()) {
		processIdentify(objectContext);
	}
	else {
		for(auto authenticationProcedure : authenticationProcedures) {
			authenticationProcedure->procedureRun(objectContext);
			if(authProperties->get().count("identified") > 0) {
				break;
			}
		}
	}
}

void RequestHandler::processRequestBasicAuth(std::set<std::string>& authDataTypes, RequestHandler::Settings& settings, esl::com::http::server::RequestContext& requestContext) const {
	const auto& headers = requestContext.getRequest().getHeaders();
	auto iter = headers.find("Authorization");
	if(iter == headers.end()) {
		return;
	}

	const std::string& header = iter->second;
	std::vector<std::string> headerSplit = esl::utility::String::split(header, ' ', true);

	if(headerSplit.size() < 1) {
		return;
	}

	if(headerSplit[0] != "Basic") {
		return;
	}

	if(headerSplit.size() < 2) {
		logger.warn << "Authorization header has no token. Header should look like \"Authorization: Basic <token>\".\n";
		return;
	}

	if(headerSplit.size() > 2) {
		logger.warn << "Authorization header too many values:  \"" << headerSplit[0];
		for(std::size_t i=1; i<headerSplit.size(); ++i) {
			logger.warn << " " << headerSplit[i];
		}
		logger.warn << "\".\n";
		logger.warn << "Authorization header should look like: \"Authorization: Basic <token>\".\n";
		logger.warn << "Drop values after <token> and continue.\n";
	}

	const std::string& token = headerSplit[1];
	std::vector<std::string> basicSplit = esl::utility::String::split( esl::utility::String::fromBase64(token), ':', false);

	if(basicSplit.size() < 1) {
		logger.warn << "Basic auth web token has no 'username'. Basic auth token should look like \"<username>:<password>\".\n";
		return;
	}

	authDataTypes.insert("basicauth");
	settings["basicauth-username"] = std::move(basicSplit[0]);

	if(basicSplit.size() < 2) {
		logger.warn << "Basic auth web token has no 'password'. Basic auth token should look like \"<username>:<password>\".\n";
	}
	else {
		settings["basicauth-password"] = std::move(basicSplit[1]);
	}

	if(basicSplit.size() >= 3) {
		logger.warn << "Basic auth web token has more arguments than required. Ignore additional arguments.\n";
	}
}

void RequestHandler::processRequestJWT(std::set<std::string>& authDataTypes, RequestHandler::Settings& settings, esl::com::http::server::RequestContext& requestContext) const {
	const auto& headers = requestContext.getRequest().getHeaders();
	auto iter = headers.find("Authorization");
	if(iter == headers.end()) {
		return;
	}

	const std::string& header = iter->second;
	std::vector<std::string> headerSplit = esl::utility::String::split(header, ' ', true);

	if(headerSplit.size() < 1) {
		return;
	}

	if(headerSplit[0] != "Bearer") {
		return;
	}

	if(headerSplit.size() < 2) {
		logger.warn << "Authorization header has no token. Header should look like \"Authorization: Bearer <token>\".\n";
		return;
	}

	if(headerSplit.size() > 2) {
		logger.warn << "Authorization header too many values:  \"" << headerSplit[0];
		for(std::size_t i=1; i<headerSplit.size(); ++i) {
			logger.warn << " " << headerSplit[i];
		}
		logger.warn << "\".\n";
		logger.warn << "Authorization header should look like: \"Authorization: Bearer <token>\".\n";
		logger.warn << "Drop values after <token> and continue.\n";
	}

	const std::string& token = headerSplit[1];
	std::vector<std::string> jwtSplit = esl::utility::String::split(token, '.', false);

	if(jwtSplit.size() < 1) {
		logger.warn << "JSON web token has no 'header'. JWT should look like \"<header>.<payload>.<signature>\".\n";
		return;
	}

	if(jwtSplit.size() < 2) {
		logger.warn << "JSON web token has no 'payload'. JWT should look like \"<header>.<payload>.<signature>\".\n";
		return;
	}

	authDataTypes.insert("jwt");
	settings["jwt-header"] =  esl::utility::String::fromBase64(jwtSplit[0]);
	settings["jwt-payload"] = esl::utility::String::fromBase64(jwtSplit[1]);
	if(jwtSplit.size() >= 3) {
		settings["jwt-signature"] = esl::utility::String::fromBase64(jwtSplit[2]);
	}
	else {
		logger.warn << "JSON web token has no 'signature'. JWT should look like \"<header>.<payload>.<signature>\".\n";
		logger.warn << "Continue without signature.\n";
	}
	settings["jwt-aud"] = requestContext.getRequest().getHostName();

	if(jwtSplit.size() > 3) {
		logger.warn << "JSON web token has no too many values: \"" << token << "\".\n";
		logger.warn << "JSON web token should look like: \"<header>.<payload>.<signature>\".\n";
		logger.warn << "Drop values after <signature> and continue.\n";
	}
}

void RequestHandler::processIdentify(esl::object::ObjectContext& objectContext) const {
	Properties* authProperties = objectContext.findObject<Properties>("authenticated");
	if(authProperties == nullptr) {
		return;
	}

	auto basicAuthIter = authProperties->get().find("basicauth-username");
	if(allows.count(basic) && basicAuthIter != authProperties->get().end()) {
		authProperties->get()["identified"] = basicAuthIter->second;
		return;
	}

	auto jwtIter = authProperties->get().find("jwt-payload");
	if(allows.count(bearer) && jwtIter != authProperties->get().end()) {
		rapidjson::Document document;
		document.Parse(jwtIter->second.c_str());

		if(document.IsObject()) {
			if(!document.HasMember("sub") || !document["sub"].IsString()) {
				authProperties->get()["identified"] = "";
			}
			else {
				authProperties->get()["identified"] = document["sub"].GetString();
			}
		}
	}
}

esl::io::Input RequestHandler::processResponse(esl::com::http::server::RequestContext& requestContext) const {
	Properties* authProperties = requestContext.getObjectContext().findObject<Properties>("authenticated");
	if(authProperties && authProperties->get().count("identified")) {
		return esl::io::Input();
	}

	if(logger.warn) {
		Properties* authProperties = requestContext.getObjectContext().findObject<Properties>("authenticated");
		if(authProperties) {
			auto basicAuthIter = authProperties->get().find("basicauth-username");
			if(basicAuthIter != authProperties->get().end()) {
				logger.warn << "Authentication failed for user \"" << basicAuthIter->second << "\"\n";
			}
		}
	}

	switch(behavior) {
	case disconnect:
		return esl::io::input::Closed::create();
	case reply:
		throw esl::com::http::server::exception::StatusCode(401);
	case proceed:
		break;
	}

	/* proceed */
	return esl::io::Input();
}

} /* namespace authentication */
} /* namespace http */
} /* namespace builtin */
} /* namespace jerry */
