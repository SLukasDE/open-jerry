/*
 * This file is part of Jerry application server.
 * Copyright (C) 2020-2021 Sven Lukas
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

#include <jerry/config/LoggerConfig.h>
#include <jerry/config/XMLException.h>

namespace jerry {
namespace config {

LoggerConfig::LoggerConfig()
: Config("")
{ }

LoggerConfig::LoggerConfig(const std::string& fileName, const tinyxml2::XMLElement& element)
: Config(fileName, element)
{
	if(element.GetUserData() != nullptr) {
		throw XMLException(*this, "Element has user data but it should be empty");
	}
	for(const tinyxml2::XMLAttribute* attribute = element.FirstAttribute(); attribute != nullptr; attribute = attribute->Next()) {
		if(std::string(attribute->Name()) == "layout") {
			layout = attribute->Value();
		}
		else {
			throw XMLException(*this, "Unknown attribute '" + std::string(attribute->Name()) + "'");
		}
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

void LoggerConfig::save(std::ostream& oStream, std::size_t spaces) const {
	oStream << makeSpaces(spaces) << "<logger layout=\"" << layout << "\">\n";
	for(const auto& entry : levelSettings) {
		entry.save(oStream, spaces+2);
	}
	for(const auto& entry : layoutSettings) {
		entry.saveLayout(oStream, spaces+2);
	}
	oStream << makeSpaces(spaces) << "</logger/>\n";
}

void LoggerConfig::parseInnerElement(const tinyxml2::XMLElement& element) {
	if(element.Name() == nullptr) {
		throw XMLException(*this, "Element name is empty");
	}

	std::string elementName(element.Name());

	if(elementName == "layout") {
		layoutSettings.push_back(Setting(getFileName(), element, false));
	}
	else if(elementName == "setting") {
		levelSettings.push_back(LevelSetting(getFileName(), element));
	}
	else {
		throw XMLException(*this, "Unknown element name \"" + elementName + "\"");
	}
}

} /* namespace config */
} /* namespace jerry */
