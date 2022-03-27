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

#include <jerry/config/application/Context.h>
#include <jerry/config/application/EntryImpl.h>
#include <jerry/config/Config.h>
#include <jerry/config/XMLException.h>
#include <jerry/engine/application/Context.h>
#include <jerry/Logger.h>

#include <esl/Module.h>

namespace jerry {
namespace config {
namespace application {

namespace {
Logger logger("jerry::config::application::Context");

std::string getConfigFile(const boost::filesystem::path& path) {
	boost::filesystem::path configFile = path / "jerry.xml";
	return configFile.generic_string();
}
} /* anonymous namespace */

Context::Context(const boost::filesystem::path& aPath)
: Config(getConfigFile(aPath)),
  appName(aPath.filename().generic_string()),
  path(aPath)
{
	tinyxml2::XMLError xmlError = xmlDocument.LoadFile(getFileName().c_str());
	if(xmlError != tinyxml2::XML_SUCCESS) {
		throw XMLException(*this, xmlError);
	}

	const tinyxml2::XMLElement* element = xmlDocument.RootElement();
	if(element == nullptr) {
		throw XMLException(*this, "No root element");
	}

	setXMLFile(getFileName(), *element);
	loadXML(*element);
}

void Context::save(std::ostream& oStream) const {
	oStream << "\n<jerry-app>\n";

	for(const auto& entry : entries) {
		entry->save(oStream, 2);
	}

	oStream << "</jerry-app>\n";
}

void Context::install(engine::main::Applications& engineApplications) {
	loadLibraries();

	std::unique_ptr<engine::application::Context> context(new engine::application::Context(appName, engineApplications.getObjectContext()));
	engine::application::Context& contextRef = *context;

	engineApplications.addApplicationContext(appName, std::move(context));

	for(const auto& entry : entries) {
		entry->install(contextRef);
	}
}

void Context::loadXML(const tinyxml2::XMLElement& element) {
	if(element.Name() == nullptr) {
		throw XMLException(*this, "Name of XML root element is empty");
	}
	if(std::string(element.Name()) != "jerry-app") {
		throw XMLException(*this, "Name of XML root element is \"" + std::string(element.Name()) + "\" but should be \"jerry-app\"");
	}
	if(element.GetUserData() != nullptr) {
		throw XMLException(*this, "Node has user data but it should be empty");
	}

	for(const tinyxml2::XMLAttribute* attribute = element.FirstAttribute(); attribute != nullptr; attribute = attribute->Next()) {
		throw XMLException(*this, "Unknown attribute '" + std::string(attribute->Name()) + "'");
	}

	for(const tinyxml2::XMLNode* node = element.FirstChild(); node != nullptr; node = node->NextSibling()) {
		const tinyxml2::XMLElement* innerElement = node->ToElement();

		if(innerElement == nullptr) {
			continue;
		}

		auto oldXmlFile = setXMLFile(getFileName(), *innerElement);
		parseInnerElement(*innerElement);
		setXMLFile(oldXmlFile);
	}
}

void Context::parseInnerElement(const tinyxml2::XMLElement& element) {
	if(element.Name() == nullptr) {
		throw XMLException(*this, "Element name is empty");
	}

	const std::string elementName(element.Name());
	if(elementName == "library") {
		parseLibrary(element);
	}
	else {
		entries.emplace_back(new EntryImpl(getFileName(), element));
	}
}

void Context::parseLibrary(const tinyxml2::XMLElement& element) {
	std::string fileName;

	if(element.GetUserData() != nullptr) {
		throw XMLException(*this, "Element has user data but it should be empty");
	}

	for(const tinyxml2::XMLAttribute* attribute = element.FirstAttribute(); attribute != nullptr; attribute = attribute->Next()) {
		if(std::string(attribute->Name()) == "file") {
			fileName = attribute->Value();
			if(fileName == "") {
				throw XMLException(*this, "Value \"\" of attribute 'file' is invalid.");
			}
		}
		else {
			throw XMLException(*this, "Unknown attribute '" + std::string(attribute->Name()) + "'");
		}
	}

	if(fileName == "") {
		throw XMLException(*this, "Missing attribute 'file'");
	}

	boost::filesystem::path libraryFileInside = path / fileName;
	boost::filesystem::path libraryFileOutside = fileName;

	if(boost::filesystem::is_regular_file(libraryFileInside)) {
		libraries.push_back(std::make_pair(libraryFileInside.generic_string(), nullptr));
	}
	else if(boost::filesystem::is_regular_file(libraryFileOutside)) {
		libraries.push_back(std::make_pair(libraryFileOutside.generic_string(), nullptr));
	}
	else {
		throw XMLException(*this, "Cannot find library-file '" + fileName + "'");
	}
}

void Context::loadLibraries() {
	/* ************************
	 * load and add libraries *
	 * ********************** */
	for(auto& library : libraries) {
		/*
		if(library.second) {
			throw esl::addStacktrace(std::runtime_error(std::string("Library \"") + library.first + "\" loaded already."));
		}
		*/
		library.second = &esl::module::Library::load(library.first);
		library.second->install(esl::getModule());
	}
}

} /* namespace application */
} /* namespace config */
} /* namespace jerry */
