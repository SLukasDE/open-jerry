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

#include <jerry/config/logging/Logger.h>
#include <jerry/config/XMLException.h>
#include <jerry/Logger.h>

#include <esl/logging/Logger.h>
#include <esl/logging/Config.h>

namespace jerry {
namespace config {
namespace logging {

Logger::Logger(const std::string& configuration)
: Config("{mem}")
{
	tinyxml2::XMLDocument xmlDocument;

	tinyxml2::XMLError xmlError = xmlDocument.Parse(configuration.c_str(), configuration.size());
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

Logger::Logger(const boost::filesystem::path& filename)
: Config(filename.generic_string())
{
	tinyxml2::XMLDocument xmlDocument;

	tinyxml2::XMLError xmlError = xmlDocument.LoadFile(filename.generic_string().c_str());
	if(xmlError != tinyxml2::XML_SUCCESS) {
		throw XMLException(*this, xmlError);
	}

	const tinyxml2::XMLElement* element = xmlDocument.RootElement();
	if(element == nullptr) {
		throw XMLException(*this, "No root element");
	}

	setXMLFile(filename.generic_string(), *element);
	loadXML(*element);
}

Logger::Logger(const std::string& filename, const tinyxml2::XMLElement& element)
: Config(filename, element)
{
	if(element.GetUserData() != nullptr) {
		throw XMLException(*this, "Element has user data but it should be empty");
	}

	for(const tinyxml2::XMLNode* node = element.FirstChild(); node != nullptr; node = node->NextSibling()) {
		const tinyxml2::XMLElement* innerElement = node->ToElement();

		if(innerElement == nullptr) {
			continue;
		}

		parseInnerElement(*innerElement);
	}
}

void Logger::save(std::ostream& oStream) const {
	oStream << "\n<esl-logger>\n";

	for(const auto& layout : layouts) {
		layout.second.save(oStream, 2);
	}

	for(const auto& appender : appenders) {
		appender.save(oStream, 2);
	}

	for(const auto& setting : settings) {
		setting.save(oStream, 2);
	}

	oStream << "</esl-logger>\n";
}

void Logger::install() const {
	/* *************** *
	 * Install layouts *
	 * *************** */
	for(const auto& layout : layouts) {
		jerry::Logger::addLayout(layout.first, layout.second.create());
	}

	/* ***************** *
	 * Install appenders *
	 * ***************** */
	for(const auto& appender : appenders) {
		jerry::Logger::addAppender(appender.getName(), appender.getLayoutId(), appender.create());
	}

	/* ***************** *
	 * Install level settings *
	 * ***************** */
	for(const auto& levelSetting : settings) {
		esl::logging::Config::setLevel(levelSetting.getLevel(), levelSetting.getScope());
	}
}

void Logger::loadXML(const tinyxml2::XMLElement& element) {
	if(element.Name() == nullptr) {
		throw XMLException(*this, "Name of XML root element is empty");
	}

	const std::string elementName(element.Name());

	if(elementName != "esl-logger") {
		throw XMLException(*this, "Name of XML root element is \"" + std::string(element.Name()) + "\" but should be \"esl-logger\"");
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

void Logger::parseInnerElement(const tinyxml2::XMLElement& element) {
	if(element.Name() == nullptr) {
		throw XMLException(*this, "Element name is empty");
	}

	std::string elementName(element.Name());

	if(elementName == "appender") {
		appenders.push_back(Appender(getFileName(), element));
	}
	else if(elementName == "layout") {
		Layout layout(getFileName(), element);
		if(layouts.count(layout.getId()) != 0) {
			throw XMLException(*this, "Multiple definition of layout with id \"" + layout.getId() + "\"");
		}
		layouts.insert(std::make_pair(layout.getId(), layout));
	}
	else if(elementName == "setting") {
		settings.push_back(LevelSetting(getFileName(), element));
	}
	else {
		throw XMLException(*this, "Unknown element name \"" + elementName + "\"");
	}
}

} /* namespace logging */
} /* namespace config */
} /* namespace jerry */
