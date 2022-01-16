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

#include <jerry/builtin/object/application/Object.h>
#include <jerry/config/application/Application.h>
#include <jerry/Logger.h>

#include <esl/Stacktrace.h>

#include <boost/filesystem.hpp>

#include <stdexcept>

namespace jerry {
namespace builtin {
namespace object {
namespace application {

namespace {
Logger logger("jerry::builtin::object::application::Object");
} /* anonymous namespace */

std::unique_ptr<esl::object::Interface::Object> Object::create(const esl::module::Interface::Settings& settings) {
	return std::unique_ptr<esl::object::Interface::Object>(new Object(settings));
}

Object::Object(const esl::module::Interface::Settings& settings) {
	for(const auto& setting : settings) {
		if(setting.first == "path") {
			if(!path.empty()) {
				throw std::runtime_error("Multiple definition of parameter 'path'.");
			}
			path = setting.second;
			if(path.empty()) {
				throw std::runtime_error("Invalid value \"\" for parameter 'path'.");
			}
			if(!boost::filesystem::exists(path)) {
				throw std::runtime_error("Path \"" + setting.second + "\" does not exists.");
			}
			if(!boost::filesystem::is_directory(path)) {
				throw std::runtime_error("Path \"" + setting.second + "\" is not a directory.");
			}
		}
		else {
			throw std::runtime_error("Unknown parameter key=\"" + setting.first + "\" with value=\"" + setting.second + "\"");
		}
	}

	if(path.empty()) {
		throw std::runtime_error("Missing specification of parameter 'path'.");
	}
}

void Object::initializeContext(esl::object::ObjectContext& parentObjectContext) {
	boost::filesystem::directory_iterator endIter;
	for( boost::filesystem::directory_iterator iter(path); iter != endIter; ++iter) {
		if(!boost::filesystem::is_directory(iter->status())) {
			continue;
		}

		boost::filesystem::path appPath = iter->path();
		boost::filesystem::path configFile = appPath / "jerry.xml";
		if(boost::filesystem::is_regular_file(configFile)) {
			try {
				config::application::Application configApp(configFile.generic_string());

				boost::filesystem::path libraryFile = appPath / "jerry.so";
				std::string libraryFileStr = boost::filesystem::is_regular_file(libraryFile) ? libraryFile.generic_string() : "";
				std::unique_ptr<Application> application(new Application(parentObjectContext, libraryFileStr));

				configApp.install(*application);
				application->initializeContext();

				applications.push_back(std::move(application));
			}
			catch(const std::exception& e) {
				logger.error << "Failed to initialize jerry-app at \"" << configFile.generic_string() << "\" because an exception has been thrown\n";
				logger.error << "What      : " << e.what() << "\n";

				const esl::Stacktrace* stacktrace = esl::getStacktrace(e);
				if(stacktrace) {
					logger.error << "Stacktrace:\n";
					stacktrace->dump(logger.error);
				}
				else {
					logger.error << "Stacktrace: not available\n";
				}
			}
			catch(...) {
				logger.error << "Failed to initialize jerry-app at \"" << configFile.generic_string() << "\" because an unknown exception has been thrown\n";
			}
		}
	}
}

const std::vector<std::unique_ptr<Application>>& Object::getApplications() const {
	return applications;
}

} /* namespace application */
} /* namespace object */
} /* namespace builtin */
} /* namespace jerry */
