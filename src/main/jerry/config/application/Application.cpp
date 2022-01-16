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

#include <jerry/config/application/Application.h>
#include <jerry/config/Config.h>
#include <jerry/config/XMLException.h>
#include <jerry/Logger.h>

#include <esl/Module.h>

namespace jerry {
namespace config {
namespace application {

namespace {
Logger logger("jerry::config::application::Application");
} /* anonymous namespace */

Application::Application(const std::string& fileName)
: Config(fileName)
{
	tinyxml2::XMLError xmlError = xmlDocument.LoadFile(fileName.c_str());
	if(xmlError != tinyxml2::XML_SUCCESS) {
		throw jerry::config::XMLException(*this, xmlError);
	}

	const tinyxml2::XMLElement* element = xmlDocument.RootElement();
	if(element == nullptr) {
		throw jerry::config::XMLException(*this, "No root element");
	}

	setXMLFile(fileName, *element);
	loadXML(*element);
}

void Application::save(std::ostream& oStream) const {
	oStream << "\n<jerry-app>\n";

	for(const auto& entry : entries) {
		entry->save(oStream, 2);
	}

	oStream << "</jerry-app>\n";
}

void Application::install(builtin::object::application::Application& engineApplication) {
	for(const auto& entry : entries) {
		entry->install(engineApplication);
	}

	if(basicListener) {
		basicListener->install(engineApplication);
	}

	if(httpListener) {
		httpListener->install(engineApplication);
	}
}

void Application::loadXML(const tinyxml2::XMLElement& element) {
	if(element.Name() == nullptr) {
		throw jerry::config::XMLException(*this, "Name of XML root element is empty");
	}
	if(std::string(element.Name()) != "jerry-app") {
		throw jerry::config::XMLException(*this, "Name of XML root element is \"" + std::string(element.Name()) + "\" but should be \"jerry-app\"");
	}
	if(element.GetUserData() != nullptr) {
		throw jerry::config::XMLException(*this, "Node has user data but it should be empty");
	}

	for(const tinyxml2::XMLAttribute* attribute = element.FirstAttribute(); attribute != nullptr; attribute = attribute->Next()) {
		throw jerry::config::XMLException(*this, "Unknown attribute '" + std::string(attribute->Name()) + "'");
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

void Application::parseInnerElement(const tinyxml2::XMLElement& element) {
	if(element.Name() == nullptr) {
		throw jerry::config::XMLException(*this, "Element name is empty");
	}

	std::string elementName(element.Name());

	if(elementName == "basic-listener") {
		if(basicListener) {
			throw jerry::config::XMLException(*this, "Multiple definition of attribute 'basic-listener' is not allowed");
		}
		basicListener.reset(new BasicListener(getFileName(), element));
	}
	else if(elementName == "http-listener") {
		if(httpListener) {
			throw jerry::config::XMLException(*this, "Multiple definition of attribute 'http-listener' is not allowed");
		}
		httpListener.reset(new HttpListener(getFileName(), element));
	}
	else {
		entries.emplace_back(new Entry(getFileName(), element));
	}
}

} /* namespace application */
} /* namespace config */
} /* namespace jerry */
