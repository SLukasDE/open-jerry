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

#include <jerry/builtin/http/application/RequestHandler.h>
#include <jerry/engine/http/RequestContext.h>
#include <jerry/engine/http/Context.h>
#include <jerry/engine/http/InputProxy.h>
#include <jerry/Logger.h>

#include <esl/com/http/server/exception/StatusCode.h>

namespace jerry {
namespace builtin {
namespace http {
namespace application {

namespace {
Logger logger("jerry::builtin::http::application::RequestHandler");
} /* anonymous namespace */


std::unique_ptr<esl::com::http::server::requesthandler::Interface::RequestHandler> RequestHandler::createRequestHandler(const esl::module::Interface::Settings& settings) {
	return std::unique_ptr<esl::com::http::server::requesthandler::Interface::RequestHandler>(new RequestHandler(settings));
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
		throw std::runtime_error("Missing specification of parameter 'engine-id'.");
	}
}

esl::io::Input RequestHandler::accept(esl::com::http::server::RequestContext& baseRequestContext) const {
	if(applications == nullptr) {
        throw esl::com::http::server::exception::StatusCode(500, "Initialization failed for 'applications-id'=\"" + applicationsId + "\"");
	}

	std::unique_ptr<jerry::engine::http::RequestContext> requestContext(new jerry::engine::http::RequestContext(baseRequestContext));

	for(auto& appsEntry : applications->getApplications()) {
		if(!appsEntry.second) {
			logger.error << "Application \"" << appsEntry.first << "\" is null\n";
			continue;
		}

		jerry::engine::http::Context* context = appsEntry.second->getHttpListener();
		if(context) {
			esl::io::Input input = context->accept(*requestContext);
			if(input) {
				return engine::http::InputProxy::create(std::move(input), std::move(requestContext));
			}
		}
	}
	return esl::io::Input();
}

void RequestHandler::initializeContext(esl::object::ObjectContext& objectContext) {
	applications = objectContext.findObject<engine::Applications>(applicationsId);
	if(applications == nullptr) {
		throw std::runtime_error("Cannot find application object with id \"" + applicationsId + "\"");
	}
}

} /* namespace application */
} /* namespace http */
} /* namespace builtin */
} /* namespace jerry */
