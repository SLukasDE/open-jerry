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

#include <jerry/config/Applications.h>
#include <jerry/config/XMLException.h>
#include <jerry/engine/Applications.h>
#include <jerry/Logger.h>

#include <boost/filesystem.hpp>

namespace jerry {
namespace config {

namespace {
Logger logger("jerry::config::Applications");
} /* anonymous namespace */

Applications::Applications(const std::string& fileName, const tinyxml2::XMLElement& element)
: Config(fileName, element)
{
	if(element.GetUserData() != nullptr) {
		throw XMLException(*this, "Element has user data but it should be empty");
	}

	for(const tinyxml2::XMLAttribute* attribute = element.FirstAttribute(); attribute != nullptr; attribute = attribute->Next()) {
		if(std::string(attribute->Name()) == "id") {
			if(!id.empty()) {
				throw XMLException(*this, "Multiple definition of attribute 'id'.");
			}
			id = attribute->Value();
			if(id.empty()) {
				throw XMLException(*this, "Value \"\" of attribute 'id' is invalid.");
			}
		}
		else if(std::string(attribute->Name()) == "path") {
			if(!path.empty()) {
				throw XMLException(*this, "Multiple definition of attribute 'path'.");
			}
			path = attribute->Value();
			if(id.empty()) {
				throw XMLException(*this, "Value \"\" of attribute 'path' is invalid.");
			}
		}
		else {
			throw XMLException(*this, "Unknown attribute '" + std::string(attribute->Name()) + "'");
		}
	}

	if(id.empty()) {
		throw XMLException(*this, "Missing attribute 'id'");
	}
	if(path.empty()) {
		throw XMLException(*this, "Missing attribute 'path'");
	}

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

		applications.emplace_back(new Application(iter->path()));
	}
}

void Applications::save(std::ostream& oStream, std::size_t spaces) const {
	oStream << makeSpaces(spaces) << "<applications id=\"" << id << "\" path=\"" << path << "\"/>\n";
}

void Applications::install(engine::ObjectContext& engineObjectContext) const {
	std::unique_ptr<engine::Applications> engineApplications(new engine::Applications(engineObjectContext));

	for(auto& applicationPtr : applications) {
		applicationPtr->install(*engineApplications);
	}

	engineObjectContext.addObject(id, std::unique_ptr<esl::object::Interface::Object>(engineApplications.release()));
}

} /* namespace config */
} /* namespace jerry */
