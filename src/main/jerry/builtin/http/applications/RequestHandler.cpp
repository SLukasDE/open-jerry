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
#include <jerry/Logger.h>

#include <stdexcept>

namespace jerry {
namespace builtin {
namespace http {
namespace applications {

namespace {
Logger logger("jerry::builtin::http::applications::RequestHandler");
} /* anonymous namespace */

std::unique_ptr<esl::com::http::server::requesthandler::Interface::RequestHandler> RequestHandler::create(const std::vector<std::pair<std::string, std::string>>& settings) {
	return std::unique_ptr<esl::com::http::server::requesthandler::Interface::RequestHandler>(new RequestHandler(settings));
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
		else if(setting.first == "context-id") {
			if(!contextId.empty()) {
				throw std::runtime_error("Multiple definition of attribute 'context-id'");
			}
			contextId = setting.second;
			if(contextId.empty()) {
				throw std::runtime_error("Invalid value \"\" for attribute 'context-id");
			}
		}
		else {
			throw std::runtime_error("Unknown parameter key=\"" + setting.first + "\" with value=\"" + setting.second + "\"");
		}
	}

	if(applicationsId.empty()) {
		throw std::runtime_error("Attribute 'applications-id' is missing");
	}

	if(applicationName.empty() && !contextId.empty()) {
		throw std::runtime_error("Cannot specify attribute 'context-id' without specifying 'application-name'");
	}
}

esl::io::Input RequestHandler::accept(esl::com::http::server::RequestContext& requestContext) const {
	if(httpContext == nullptr && procedureContext == nullptr) {
		if(procedureContext) {
			procedureContext->procedureRun(requestContext.getObjectContext());
		}

		/* TODO: RequestContext musss in engine::http::RequestContext umgewandelt werden
		if(httpContext) {
			esl::io::Input input = httpContext->accept(requestContext);
			if(input) {
				return input;
			}
		}
		*/
	}
	else {

	}

	for(auto& appsEntry : applications->getApplications()) {
		appsEntry.second->getProcedureContext().procedureRun(requestContext.getObjectContext());

		/* TODO: RequestContext musss in engine::http::RequestContext umgewandelt werden
		esl::io::Input input = appsEntry.second->getHttpContext().accept(requestContext);
		if(input) {
			return input;
		}
		*/
	}
}

void RequestHandler::initializeContext(esl::object::ObjectContext& objectContext) {
	applications = objectContext.findObject<engine::main::Applications>(applicationsId);
	if(applications == nullptr) {
		throw std::runtime_error("Cannot find applications object with id \"" + applicationsId + "\"");
	}
	if(!applicationName.empty()) {
		auto iter = applications->getApplications().find(applicationName);
		if(iter == applications->getApplications().end()) {
			throw std::runtime_error("Application name '" + applicationName + "' not available in applications object with id '" + applicationsId + "'");
		}
		engine::application::Context* application = iter->second.get();
		if(application == nullptr) {
			throw std::runtime_error("Engine error");
		}

		if(contextId.empty()) {
			httpContext = &application->getHttpContext();
			procedureContext = &application->getProcedureContext();
		}
		else {
			httpContext = application->findObject<engine::http::Context>(contextId);
			procedureContext = application->findObject<engine::procedure::Context>(contextId);
			if(httpContext == nullptr && procedureContext == nullptr) {
				throw std::runtime_error("Context id '" + contextId + "' not available in applications object with id '" + applicationsId + "' and application name '" + applicationName + "'");
			}
		}
	}
}

} /* namespace applications */
} /* namespace http */
} /* namespace builtin */
} /* namespace jerry */
