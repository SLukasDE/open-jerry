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

#include <jerry/builtin/procedure/authorization/cache/Procedure.h>
#include <jerry/Logger.h>

#include <esl/utility/String.h>

#include <stdexcept>

namespace jerry {
namespace builtin {
namespace procedure {
namespace authorization {
namespace cache {

namespace {
Logger logger("jerry::builtin::procedure::authorization::cache::Procedure");
} /* anonymous namespace */

std::unique_ptr<esl::processing::Procedure> Procedure::create(const std::vector<std::pair<std::string, std::string>>& settings) {
	return std::unique_ptr<esl::processing::Procedure>(new Procedure(settings));
}

Procedure::Procedure(const std::vector<std::pair<std::string, std::string>>& settings) {
	bool hasAuthorizedObjectId = false;
	bool hasLifetimeRenew = false;

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
		else if(setting.first == "authorizing-procedure-id") {
			if(!authorizingProcedureId.empty()) {
				throw std::runtime_error("Multiple definition of attribute 'authorizing-procedure-id'");
			}
			authorizingProcedureId = setting.second;
			if(authorizingProcedureId.empty()) {
				throw std::runtime_error("Invalid value \"\" for attribute 'authorizing-procedure-id'");
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

	if(lifetimeMs == std::chrono::milliseconds(0)) {
		throw std::runtime_error("Parameter 'lifetime-ms' is missing");
	}
}

void Procedure::procedureRun(esl::object::Context& objectContext) {
	/* we are done just for the case a previous procedure created already an authorization object */
	if(objectContext.findObject<esl::object::Object>(authorizedObjectId)) {
		return;
	}

	/* get the identifier string to lookup for an authorization object in out session pool */
	Properties* authProperties = objectContext.findObject<Properties>("authenticated");
	if(!authProperties) {
		return;
	}

	auto iter = authProperties->get().find("identified");
	if(iter == authProperties->get().end()) {
		logger.warn << "Cannot lookup for authorization without identified user\n.";
		return;
	}
	const std::string& user = iter->second;

	/* lookup for an authorization object in our session pool */
	auto object = sessionPool->get(user, objectContext);

	/* we are done if a new authorized object has been created because it would have been created in our object context and a copy was created to store in our session pool */
	if(objectContext.findObject<esl::object::Object>(authorizedObjectId)) {
		return;
	}

	/* if no object has been created then sessionPool-get(...) should have returned the existing object */
	if(!object) {
		logger.warn << "No authorization object found available for user \"" << user << "\"\n.";
		return;
	}

	/* the object should be give back into the session pool, so we have to clone it to put it into our object context */
	esl::object::Cloneable* authorizedObject = object.get();
	if(!authorizedObject) {
		logger.warn << "Authorization object for user \"" << user << "\" is null\n.";
		return;
	}

	objectContext.addObject(authorizedObjectId, authorizedObject->clone());
}

void Procedure::procedureCancel() {
}

void Procedure::initializeContext(esl::object::Context& objectContext) {
	authorizingProcedure = objectContext.findObject<esl::processing::Procedure>(authorizingProcedureId);
	if(authorizingProcedure == nullptr) {
		throw std::runtime_error("Cannot find procedure with id \"" + authorizingProcedureId + "\"");
	}

	sessionPool.reset(new SessionPool([this](const esl::object::Context& objectContext) {
		return createAuthorizationObject(objectContext);
	}, 10, lifetimeMs, lifetimeRenew, false));
}

std::unique_ptr<esl::object::Cloneable> Procedure::createAuthorizationObject(const esl::object::Context& objectContext) {
	/* call procedure to create a new authorization object with id 'authorizedObjectId' */
	authorizingProcedure->procedureRun(const_cast<esl::object::Context&>(objectContext));

	/* lookup for object with id 'authorizedObjectId' */
	const esl::object::Object* authorizationObjectPtr = objectContext.findObject<esl::object::Object>(authorizedObjectId);

	/* check if authorization object was created */
	if(authorizationObjectPtr == nullptr) {
		logger.warn << "No authorization object was created with id \"" + authorizedObjectId + "\"\n";
		return nullptr;
	}

	/* check if authorization object is clonable */
	const esl::object::Cloneable* cloneableAuthorizationObjectPtr = dynamic_cast<const esl::object::Cloneable*>(authorizationObjectPtr);
	if(cloneableAuthorizationObjectPtr == nullptr) {
		logger.warn << "Authorization object with id \"" + authorizedObjectId + "\" was created, but it is not cloneable.\n";
		logger.warn << "Cannot store authorization object in session pool.\n";
		return nullptr;
	}

	/* return cloned object */
	return std::unique_ptr<esl::object::Cloneable>(dynamic_cast<esl::object::Cloneable*>(cloneableAuthorizationObjectPtr->clone().release()));
}

} /* namespace cache */
} /* namespace authorization */
} /* namespace procedure */
} /* namespace builtin */
} /* namespace jerry */
