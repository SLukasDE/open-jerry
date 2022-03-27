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

#include <jerry/builtin/basic/application/RequestHandler.h>
#include <jerry/engine/basic/Context.h>
#include <jerry/engine/InputProxy.h>
#include <jerry/Logger.h>

#include <stdexcept>

namespace jerry {
namespace builtin {
namespace basic {
namespace application {

namespace {
Logger logger("jerry::builtin::basic::application::RequestHandler");
} /* anonymous namespace */

std::unique_ptr<esl::com::basic::server::requesthandler::Interface::RequestHandler> RequestHandler::createRequestHandler(const esl::module::Interface::Settings& settings) {
	return std::unique_ptr<esl::com::basic::server::requesthandler::Interface::RequestHandler>(new RequestHandler(settings));
}

RequestHandler::RequestHandler(const esl::module::Interface::Settings& settings) {
	for(const auto& setting : settings) {
		if(setting.first == "applications-id") {
			if(!applicationsId.empty()) {
				throw std::runtime_error("Multiple definition of parameter 'applications-id'.");
			}
			applicationsId = setting.second;
			if(applicationsId.empty()) {
				throw std::runtime_error("Invalid value \"\" for parameter 'applications-id'.");
			}
		}
		else {
			throw std::runtime_error("Unknown parameter key=\"" + setting.first + "\" with value=\"" + setting.second + "\"");
		}
	}

	if(applicationsId.empty()) {
		throw std::runtime_error("Missing specification of parameter 'applications-id'.");
	}
}


esl::io::Input RequestHandler::accept(esl::com::basic::server::RequestContext& requestContext) const {
	if(applications == nullptr) {
        throw std::runtime_error("Initialization failed for 'applications-id'=\"" + applicationsId + "\"");
	}

	for(auto& appsEntry : applications->getApplications()) {
		if(!appsEntry.second) {
			logger.error << "Application \"" << appsEntry.first << "\" is null\n";
			continue;
		}

		esl::io::Input input = appsEntry.second->getBasicContext().accept(requestContext);
		if(input) {
			return engine::InputProxy::create(std::move(input));
		}
	}
	return esl::io::Input();
}

std::set<std::string> RequestHandler::getNotifiers() const {
	return std::set<std::string>();
}

void RequestHandler::initializeContext(esl::object::ObjectContext& objectContext) {
	applications = objectContext.findObject<engine::main::Applications>(applicationsId);
	if(applications == nullptr) {
		throw std::runtime_error("Cannot find application object with id \"" + applicationsId + "\"");
	}
}

} /* namespace application */
} /* namespace basic */
} /* namespace builtin */
} /* namespace jerry */
