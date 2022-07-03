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

#include <jerry/builtin/http/applications/RequestHandler.h>
#include <jerry/engine/http/Context.h>
#include <jerry/engine/procedure/Context.h>
#include <jerry/Logger.h>

#include <esl/com/http/server/exception/StatusCode.h>
#include <esl/processing/Procedure.h>

#include <stdexcept>
//#include <exception>

namespace jerry {
namespace builtin {
namespace http {
namespace applications {

namespace {
Logger logger("jerry::builtin::http::applications::RequestHandler");
} /* anonymous namespace */

std::unique_ptr<esl::com::http::server::RequestHandler> RequestHandler::create(const std::vector<std::pair<std::string, std::string>>& settings) {
	return std::unique_ptr<esl::com::http::server::RequestHandler>(new RequestHandler(settings));
}

RequestHandler::RequestHandler(const std::vector<std::pair<std::string, std::string>>& settings) {
	for(const auto& setting : settings) {
		if(setting.first == "applications-id") {
			if(!applicationsId.empty()) {
				throw std::runtime_error("Multiple definition of attribute 'applications-id'");
			}
			applicationsId = setting.second;
			if(applicationsId.empty()) {
				throw std::runtime_error("Invalid value \"\" for attribute 'applications-id'");
			}
		}
		else if(setting.first == "application-name") {
			if(!applicationName.empty()) {
				throw std::runtime_error("Multiple definition of attribute 'application-name'");
			}
			applicationName = setting.second;
			if(applicationName.empty()) {
				throw std::runtime_error("Invalid value \"\" for attribute 'application-name'");
			}
		}
		else if(setting.first == "ref-id") {
			if(!refId.empty()) {
				throw std::runtime_error("Multiple definition of attribute 'ref-id'");
			}
			refId = setting.second;
			if(refId.empty()) {
				throw std::runtime_error("Invalid value \"\" for attribute 'ref-id");
			}
		}
		else {
			throw std::runtime_error("Unknown parameter key=\"" + setting.first + "\" with value=\"" + setting.second + "\"");
		}
	}

	if(applicationsId.empty()) {
		throw std::runtime_error("Attribute 'applications-id' is missing");
	}

	if(applicationName.empty() && !refId.empty()) {
		throw std::runtime_error("Cannot specify attribute 'ref-id' without specifying 'application-name'");
	}
}

esl::io::Input RequestHandler::accept(esl::com::http::server::RequestContext& requestContext) const {
	return applications->accept(requestContext, application, refObject);
}

void RequestHandler::initializeContext(esl::object::Context& objectContext) {
	applications = objectContext.findObject<object::applications::Object>(applicationsId);
	if(applications == nullptr) {
		throw std::runtime_error("Cannot find applications object with id \"" + applicationsId + "\"");
	}

	if(!applicationName.empty()) {
		auto iter = applications->getApplications().find(applicationName);
		if(iter == applications->getApplications().end()) {
			throw std::runtime_error("Application name '" + applicationName + "' not available in applications object with id '" + applicationsId + "'");
		}
		application = iter->second.get();
		if(application == nullptr) {
			throw std::runtime_error("Engine error");
		}

		if(!refId.empty()) {
			refObject = application->findObject<esl::object::Object>(refId);
			if(refObject) {
				throw std::runtime_error("Reference id '" + refId + "' not available in applications object with id '" + applicationsId + "' and application name '" + applicationName + "'");
			}

			if(dynamic_cast<engine::http::Context*>(refObject) == nullptr
			&& dynamic_cast<engine::procedure::Context*>(refObject) == nullptr
			&& dynamic_cast<esl::processing::Procedure*>(refObject)) {
				throw std::runtime_error("Reference id '" + refId + "' has been found in applications object with id '" + applicationsId + "' and application name '" + applicationName + "', but type is not compatible");
			}
		}
	}
}

} /* namespace applications */
} /* namespace http */
} /* namespace builtin */
} /* namespace jerry */
