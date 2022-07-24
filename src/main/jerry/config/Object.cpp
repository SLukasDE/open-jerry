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

#include <jerry/config/Object.h>
#include <jerry/config/FilePosition.h>

#include <esl/plugin/exception/PluginNotFound.h>
#include <esl/plugin/Registry.h>

#include <utility>

namespace jerry {
namespace config {

Object::Object(const std::string& fileName, const tinyxml2::XMLElement& element)
: Config(fileName, element)
{
	if(element.GetUserData() != nullptr) {
		throw FilePosition::add(*this, "Element has user data but it should be empty");
	}

	for(const tinyxml2::XMLAttribute* attribute = element.FirstAttribute(); attribute != nullptr; attribute = attribute->Next()) {
		if(std::string(attribute->Name()) == "id") {
			if(!id.empty()) {
				throw FilePosition::add(*this, "Multiple definitions of attribute 'id'.");
			}
			id = attribute->Value();
			if(id.empty()) {
				throw FilePosition::add(*this, "Invalid value \"\" for attribute 'id'");
			}
		}
		else if(std::string(attribute->Name()) == "implementation") {
			if(!implementation.empty()) {
				throw std::runtime_error("Multiple definition of attribute 'implementation'.");
			}
			implementation = attribute->Value();
			if(implementation.empty()) {
				throw FilePosition::add(*this, "Invalid value \"\" for attribute 'implementation'");
			}
		}
		else {
			throw FilePosition::add(*this, "Unknown attribute '" + std::string(attribute->Name()) + "'");
		}
	}

	if(id.empty()) {
		throw FilePosition::add(*this, "Missing attribute 'id'");
	}
	if(implementation.empty()) {
		throw FilePosition::add(*this, "Missing attribute 'implementation'");
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

void Object::save(std::ostream& oStream, std::size_t spaces) const {
	oStream << makeSpaces(spaces) << "<object id=\"" << id << "\" implementation=\"" << implementation << "\">\n";

	for(const auto& entry : settings) {
		entry.saveParameter(oStream, spaces+2);
	}
	oStream << makeSpaces(spaces) << "</object>\n";
}

void Object::install(esl::object::Context& engineObjectContext) const {
	engineObjectContext.addObject(id, create());
}

std::unique_ptr<esl::object::Object> Object::create() const {
	std::vector<std::pair<std::string, std::string>> eslSettings;
	for(const auto& setting : settings) {
		eslSettings.push_back(std::make_pair(setting.key, evaluate(setting.value, setting.language)));
	}

	std::unique_ptr<esl::object::Object> eslObject;
	try {
		eslObject = esl::plugin::Registry::get().create<esl::object::Object>(implementation, eslSettings);
	}
	catch(const esl::plugin::exception::PluginNotFound& e) {
		throw FilePosition::add(*this, e);
	}
	catch(const std::runtime_error& e) {
		throw FilePosition::add(*this, e);
	}
	catch(const std::exception& e) {
		throw FilePosition::add(*this, e);
	}
	catch(...) {
		throw FilePosition::add(*this, "Could not create an object with id '" + id + "' for implementation '" + implementation + "' because an unknown exception occurred.");
	}

	if(!eslObject) {
		throw FilePosition::add(*this, "Could not create an object with id '" + id + "' for implementation '" + implementation + "'");
	}

	return eslObject;
}

void Object::parseInnerElement(const tinyxml2::XMLElement& element) {
	if(element.Name() == nullptr) {
		throw FilePosition::add(*this, "Element name is empty");
	}

	std::string elementName(element.Name());

	if(elementName == "parameter") {
		settings.push_back(Setting(getFileName(), element, true));
	}
	else {
		throw FilePosition::add(*this, "Unknown element name \"" + elementName + "\"");
	}
}

} /* namespace config */
} /* namespace jerry */
