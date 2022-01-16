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

#include <jerry/builtin/http/basicauth/request/RequestHandler.h>
#include <jerry/Logger.h>

#include <esl/com/http/server/Request.h>
#include <esl/io/Input.h>
#include <esl/module/Interface.h>
#include <esl/object/Properties.h>
#include <esl/object/Interface.h>

#include <stdexcept>

namespace jerry {
namespace builtin {
namespace http {
namespace basicauth {
namespace request {

namespace {
Logger logger("jerry::builtin::http::basicauth::request::RequestHandler");
} /* anonymous namespace */

std::unique_ptr<esl::com::http::server::requesthandler::Interface::RequestHandler> RequestHandler::createRequestHandler(const esl::module::Interface::Settings& settings) {
	return std::unique_ptr<esl::com::http::server::requesthandler::Interface::RequestHandler>(new RequestHandler(settings));
}

RequestHandler::RequestHandler(const esl::module::Interface::Settings& settings) {
	for(const auto& setting : settings) {
		if(setting.first == "procedure-id") {
			if(!procedureId.empty()) {
				throw std::runtime_error("Multiple definition of attribute 'procedure-id'");
			}
			procedureId = setting.second;
			if(procedureId.empty()) {
				throw std::runtime_error("Invalid value \"\" for attribute 'procedure-id'");
			}
		}
		else {
			throw std::runtime_error("Unknown parameter key=\"" + setting.first + "\" with value=\"" + setting.second + "\"");
		}
	}

	if(procedureId.empty()) {
		throw std::runtime_error("Missing attribute 'procedure-id'");
	}
}

esl::io::Input RequestHandler::accept(esl::com::http::server::RequestContext& requestContext) const {
	esl::module::Interface::Settings settings;
	settings.push_back(std::make_pair("type", "basicauth"));
	settings.push_back(std::make_pair("username", requestContext.getRequest().getUsername()));
	settings.push_back(std::make_pair("password", requestContext.getRequest().getPassword()));

	std::unique_ptr<esl::object::Interface::Object> properties(new esl::object::Properties(settings));
	requestContext.getObjectContext().addObject("auth", std::move(properties));

	if(procedure) {
		procedure->procedureRun(requestContext.getObjectContext());
	}

	return esl::io::Input();
}

void RequestHandler::initializeContext(esl::object::ObjectContext& objectContext) {
	procedure = objectContext.findObject<esl::processing::procedure::Interface::Procedure>(procedureId);
	if(procedure == nullptr) {
		throw std::runtime_error("Cannot find procedure with id \"" + procedureId + "\"");
	}
}

} /* namespace request */
} /* namespace basicauth */
} /* namespace http */
} /* namespace builtin */
} /* namespace jerry */
