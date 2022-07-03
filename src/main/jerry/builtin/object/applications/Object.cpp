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

#include <jerry/builtin/object/applications/Object.h>
#include <jerry/builtin/object/applications/xmlconfig/Context.h>
#include <jerry/engine/InputProxy.h>
#include <jerry/engine/http/RequestContext.h>
#include <jerry/engine/http/InputProxy.h>
#include <jerry/Logger.h>

#include <esl/com/http/server/RequestContext.h>

#include <boost/filesystem.hpp>

namespace jerry {
namespace builtin {
namespace object {
namespace applications {
namespace {
Logger logger("jerry::builtin::object::applications::Object");
} /* anonymous namespace */

std::unique_ptr<esl::object::Object> Object::create(const std::vector<std::pair<std::string, std::string>>& settings) {
	return std::unique_ptr<esl::object::Object>(new Object(settings));
}


Object::Object(const std::vector<std::pair<std::string, std::string>>& settings) {
	for(const auto& setting : settings) {
		if(setting.first == "path") {
			if(!path.empty()) {
				throw std::runtime_error("Multiple definition of attribute 'path'");
			}
			path = setting.second;
			if(path.empty()) {
				throw std::runtime_error("Invalid value \"\" for attribute 'path'");
			}
		}
		else {
			throw std::runtime_error("Unknown parameter key=\"" + setting.first + "\" with value=\"" + setting.second + "\"");
		}
	}

	if(path.empty()) {
		throw std::runtime_error("Missing attribute 'path'");
	}
}

const std::map<std::string, std::unique_ptr<Application>>& Object::getApplications() const noexcept {
	return applications;
}
/*
engine::ObjectContext& Object::getObjectContext() const noexcept {
	return *objectContext;
}
*/
void Object::initializeContext(esl::object::Context& aContext) {
	objectContext = dynamic_cast<engine::ObjectContext*>(&aContext);
	if(objectContext == nullptr) {
		throw std::runtime_error("Initialization error: Object context is not an engine object context");
	}

	scan();
}

esl::io::Input Object::accept(esl::com::http::server::RequestContext& baseRequestContext, const Application* application, const esl::object::Object* object) const {
	std::unique_ptr<engine::http::RequestContext> requestContext(new engine::http::RequestContext(baseRequestContext));

//	try {
		for(auto& applicationEntry : applications) {
			if(application && application != applicationEntry.second.get()) {
				continue;
			}

			esl::io::Input input = applicationEntry.second->accept(*requestContext, object);
			if(input) {
				return engine::http::InputProxy::create(std::move(input), std::move(requestContext));
			}
		}
/*
	}
	catch(...) {
		engine::http::ExceptionHandler exceptionHandler(std::current_exception());
    	exceptionHandler.dump(logger.error);
		exceptionHandler.dumpHttp(requestContext->getConnection(), requestContext->getErrorHandlingContext(), requestContext->getHeadersContext());
	}
*/
	return esl::io::Input();
}

esl::io::Input Object::accept(esl::com::basic::server::RequestContext& requestContext, const Application* application, const esl::object::Object* object) const {
	for(auto& applicationEntry : applications) {
		if(application && application != applicationEntry.second.get()) {
			continue;
		}

		esl::io::Input input = applicationEntry.second->accept(requestContext, object);
		if(input) {
			return engine::InputProxy::create(std::move(input));
		}
	}

	return esl::io::Input();
}

void Object::procedureRun(esl::object::Context& context, const Application* application, const esl::object::Object* object) const {
	for(auto& applicationEntry : applications) {
		if(application && application != applicationEntry.second.get()) {
			continue;
		}

		applicationEntry.second->procedureRun(context, object);
	}
}

void Object::procedureCancel() const {

}

void Object::scan() {
	boost::filesystem::directory_iterator endIter;
	for( boost::filesystem::directory_iterator iter(path); iter != endIter; ++iter) {
		if(!boost::filesystem::is_directory(iter->status())) {
			continue;
		}


		boost::filesystem::path configFile = iter->path() / "jerry.xml";
		if(!boost::filesystem::is_regular_file(configFile)) {
			logger.warn << "File \"" << configFile.generic_string() << "\" not found\n";
			continue;
		}

		std::unique_ptr<Application> application(new Application(iter->path().filename().generic_string(), objectContext->getProcessRegistry()));
		Application& applicationRef = *application;
		applicationRef.setParent(objectContext);


		xmlconfig::Context configContext(iter->path());
		configContext.install(applicationRef);

		applicationRef.initializeContext();

		applications.insert(std::make_pair(applicationRef.getName(), std::move(application)));

	}
}

} /* namespace applications */
} /* namespace object */
} /* namespace builtin */
} /* namespace jerry */
