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

#include <jerry/builtin/object/applications/Application.h>
#include <jerry/builtin/object/applications/EntryImpl.h>
#include <jerry/engine/ProcessRegistry.h>
#include <jerry/Logger.h>

namespace jerry {
namespace builtin {
namespace object {
namespace applications {

namespace {
Logger logger("jerry::builtin::object::applications::Application");

std::string getConfigFile(const boost::filesystem::path& path) {
	boost::filesystem::path configFile = path / "jerry.xml";
	return configFile.generic_string();
}
} /* anonymous namespace */

Application::Application(const std::string& aName, engine::ProcessRegistry* processRegistry)
: engine::ObjectContext(processRegistry),
  name(aName)
{ }

const std::string& Application::getName() const noexcept {
	return name;
}

void Application::initializeContext() {
	engine::ObjectContext::initializeContext();

	// call initializeContext() of sub-context's
	for(auto& entry : entries) {
		entry->initializeContext(*this);
	}
}

void Application::addProcedure(std::unique_ptr<esl::processing::procedure::Interface::Procedure> procedure) {
	entries.emplace_back(new EntryImpl(std::move(procedure)));
}

void Application::addProcedure(const std::string& refId) {
	esl::processing::procedure::Interface::Procedure* procedure = findObject<esl::processing::procedure::Interface::Procedure>(refId);

	if(procedure == nullptr) {
	    throw std::runtime_error("No procedure found with ref-id=\"" + refId + "\".");
	}

	entries.emplace_back(new EntryImpl(*procedure));
}

void Application::addProcedureContext(std::unique_ptr<engine::procedure::Context> procedureContext) {
	entries.emplace_back(new EntryImpl(std::move(procedureContext)));
}

void Application::addProcedureContext(const std::string& refId) {
	engine::procedure::Context* context = findObject<engine::procedure::Context>(refId);

	if(context == nullptr) {
	    throw std::runtime_error("No procedure context found with ref-id=\"" + refId + "\".");
	}

	entries.emplace_back(new EntryImpl(*context));
}

void Application::addBasicContext(std::unique_ptr<engine::basic::Context> basicContext) {
	entries.emplace_back(new EntryImpl(std::move(basicContext)));
}

void Application::addBasicContext(const std::string& refId) {
	engine::basic::Context* context = findObject<engine::basic::Context>(refId);

	if(context == nullptr) {
	    throw std::runtime_error("No basic context found with ref-id=\"" + refId + "\".");
	}

	entries.emplace_back(new EntryImpl(*context));
}

void Application::addHttpContext(std::unique_ptr<engine::http::Context> httpContext) {
	entries.emplace_back(new EntryImpl(std::move(httpContext)));
}

void Application::addHttpContext(const std::string& refId) {
	engine::http::Context* context = findObject<engine::http::Context>(refId);

	if(context == nullptr) {
	    throw std::runtime_error("No http context found with ref-id=\"" + refId + "\".");
	}

	entries.emplace_back(new EntryImpl(*context));
}

esl::io::Input Application::accept(engine::http::RequestContext& requestContext, const esl::object::Interface::Object* object) const {
	esl::io::Input input;

	for(auto& entry : entries) {
		input = entry->accept(requestContext, object);
		if(input) {
			break;
		}
	}

	return input;
}

esl::io::Input Application::accept(esl::com::basic::server::RequestContext& requestContext, const esl::object::Interface::Object* object) const {
	esl::io::Input input;

	for(auto& entry : entries) {
		input = entry->accept(requestContext, object);
		if(input) {
			break;
		}
	}

	return input;
}

void Application::procedureRun(esl::object::ObjectContext& objectContext, const esl::object::Interface::Object* object) const {
	for(auto& entry : entries) {
		entry->procedureRun(objectContext, object);
	}
}

void Application::procedureCancel() const {
	for(auto& entry : entries) {
		entry->procedureCancel();
	}
}

} /* namespace applications */
} /* namespace object */
} /* namespace builtin */
} /* namespace jerry */
