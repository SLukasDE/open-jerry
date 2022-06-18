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

#include <jerry/config/logging/Appender.h>
#include <jerry/config/XMLException.h>

#include <esl/logging/appender/Appender.h>
#include <esl/utility/String.h>

#include <stdexcept>
#include <iostream>
#include <utility>

namespace jerry {
namespace config {
namespace logging {

Appender::Appender(const std::string& fileName, const tinyxml2::XMLElement& element)
: Config(fileName, element)
{
	if(element.GetUserData() != nullptr) {
		throw XMLException(*this, "Element has user data but it should be empty");
	}

	bool recordDefined = false;

	for(const tinyxml2::XMLAttribute* attribute = element.FirstAttribute(); attribute != nullptr; attribute = attribute->Next()) {
		if(std::string(attribute->Name()) == "name") {
			if(!name.empty()) {
				throw XMLException(*this, "Multiple definition of attribute 'name'");
			}
			name = esl::utility::String::toLower(attribute->Value());
			if(name.empty()) {
				throw XMLException(*this, "Value \"\" of attribute 'name' is invalid.");
			}
		}
		else if(std::string(attribute->Name()) == "record") {
			if(recordDefined) {
				throw XMLException(*this, "Multiple definition of attribute 'record'");
			}
			std::string recordLevelStr = esl::utility::String::toUpper(attribute->Value());

			if(recordLevelStr == "ALL") {
				recordLevel = esl::logging::appender::Interface::Appender::RecordLevel::ALL;
			}
			else if(recordLevelStr == "SELECTED") {
				recordLevel = esl::logging::appender::Interface::Appender::RecordLevel::SELECTED;
			}
			else if(recordLevelStr == "OFF") {
				recordLevel = esl::logging::appender::Interface::Appender::RecordLevel::OFF;
			}
			else {
				throw XMLException(*this, "Value \"" + std::string(attribute->Value()) + "\" of attribute 'record' is invalid. "
						"Valid values are \"all\", \"selected\" and \"off\"");
			}
			recordDefined = true;
		}
		else if(std::string(attribute->Name()) == "implementation") {
			if(implementation != "") {
				throw XMLException(*this, "Multiple definition of attribute 'implementation'");
			}
			implementation = attribute->Value();
			if(implementation == "") {
				throw XMLException(*this, "Invalid value \"\" for attribute 'implementation'");
			}
		}
		else if(std::string(attribute->Name()) == "layout") {
			if(layoutId != "") {
				throw XMLException(*this, "Multiple definition of attribute 'layout'");
			}
			layoutId = attribute->Value();
			if(layoutId == "") {
				throw XMLException(*this, "Invalid value \"\" for attribute 'layout'");
			}
		}
		else {
			throw XMLException(*this, "Unknown attribute '" + std::string(attribute->Name()) + "'");
		}
	}

	if(layoutId == "") {
		throw XMLException(*this, "Missing attribute 'layout'");
	}
	if(implementation == "") {
		throw XMLException(*this, "Missing attribute 'implementation'");
	}

	for(const tinyxml2::XMLNode* node = element.FirstChild(); node != nullptr; node = node->NextSibling()) {
		const tinyxml2::XMLElement* innerElement = node->ToElement();

		if(innerElement == nullptr) {
			continue;
		}

		parseInnerElement(*innerElement);
	}
}

const std::string& Appender::getName() const {
	return name;
}

const std::string& Appender::getLayoutId() const {
	return layoutId;
}

void Appender::save(std::ostream& oStream, std::size_t spaces) const {
	oStream << makeSpaces(spaces) << "<appender";

	if(!name.empty()) {
		oStream << " name=\"" << name << "\"";
	}

	if(!implementation.empty()) {
		oStream << " implementation=\"" << implementation << "\"";
	}

	switch(recordLevel) {
	case esl::logging::appender::Interface::Appender::RecordLevel::ALL:
		oStream << " record=\"ALL\"";
		break;
	case esl::logging::appender::Interface::Appender::RecordLevel::SELECTED:
		oStream << " record=\"SELECTED\"";
		break;
	case esl::logging::appender::Interface::Appender::RecordLevel::OFF:
		oStream << " record=\"OFF\"";
		break;
	}

	oStream << " layout=\"" << layoutId << "\"";

	if(parameters.empty()) {
		oStream << "/>\n";
	}
	else {
		oStream << ">\n";
		for(const auto& parameter : parameters) {
			parameter.saveParameter(oStream, spaces+2);
		}
		oStream << makeSpaces(spaces) << "</appender>\n";
	}
}

std::unique_ptr<esl::logging::appender::Interface::Appender> Appender::create() const {
	std::vector<std::pair<std::string, std::string>> eslSettings;
	for(auto const& setting : parameters) {
		eslSettings.push_back(std::make_pair(setting.key, setting.value));
	}

	std::unique_ptr<esl::logging::appender::Interface::Appender> appender(new esl::logging::appender::Appender(eslSettings, implementation));
	appender->setRecordLevel(recordLevel);

	return appender;
}

void Appender::parseInnerElement(const tinyxml2::XMLElement& element) {
	if(element.Name() == nullptr) {
		throw XMLException(*this, "Element name is empty");
	}

	std::string elementName(element.Name());

	if(elementName == "parameter") {
		parameters.push_back(Setting(getFileName(), element, false));
	}
	else {
		throw XMLException(*this, "Unknown element name \"" + elementName + "\"");
	}
}

} /* namespace logging */
} /* namespace config */
} /* namespace jerry */
