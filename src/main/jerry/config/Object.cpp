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
#include <jerry/config/XMLException.h>

#include <esl/module/Interface.h>

#include <memory>

namespace jerry {
namespace config {

Object::Object(const std::string& fileName, const tinyxml2::XMLElement& element)
: Config(fileName, element)
{
	if(element.GetUserData() != nullptr) {
		throw XMLException(*this, "Element has user data but it should be empty");
	}

	for(const tinyxml2::XMLAttribute* attribute = element.FirstAttribute(); attribute != nullptr; attribute = attribute->Next()) {
		if(std::string(attribute->Name()) == "id") {
			id = attribute->Value();
			if(id == "") {
				throw XMLException(*this, "Value \"\" of attribute 'id' is invalid");
			}
		}
		else if(std::string(attribute->Name()) == "implementation") {
			implementation = attribute->Value();
			if(implementation == "") {
				throw XMLException(*this, "Value \"\" of attribute 'implementation' is invalid");
			}
		}
		else {
			throw XMLException(*this, "Unknown attribute '" + std::string(attribute->Name()) + "'");
		}
	}

	if(id == "") {
		throw XMLException(*this, "Missing attribute 'id'");
	}
	if(implementation == "") {
		throw XMLException(*this, "Missing attribute 'implementation'");
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

void Object::install(engine::ObjectContext& engineObjectContext) const {
	engineObjectContext.addObject(id, install());
}

void Object::install(engine::Application& engineApplication) const {
	std::unique_ptr<esl::object::Interface::Object> eslObject = install();
	engineApplication.getLocalObjectContext().addReference(id, *eslObject);
	engineApplication.addObject(id, std::move(eslObject));
}

std::unique_ptr<esl::object::Interface::Object> Object::install() const {
	esl::module::Interface::Settings eslSettings;
	for(const auto& setting : settings) {
		eslSettings.push_back(std::make_pair(setting.key, evaluate(setting.value, setting.language)));
	}

	std::unique_ptr<esl::object::Interface::Object> eslObject;
	try {
		eslObject = esl::getModule().getInterface<esl::object::Interface>(implementation).createObject(eslSettings);
	}
	catch(const std::exception& e) {
		throw XMLException(*this, e.what());
	}
	catch(...) {
		throw XMLException(*this, "Could not create an object with id '" + id + "' for implementation '" + implementation + "' because an unknown exception occurred.");
	}

	if(!eslObject) {
		throw XMLException(*this, "Could not create an object with id '" + id + "' for implementation '" + implementation + "'");
	}

	return eslObject;
}

void Object::parseInnerElement(const tinyxml2::XMLElement& element) {
	if(element.Name() == nullptr) {
		throw XMLException(*this, "Element name is empty");
	}

	std::string elementName(element.Name());

	if(elementName == "parameter") {
		settings.push_back(Setting(getFileName(), element, true));
	}
	else {
		throw XMLException(*this, "Unknown element name \"" + elementName + "\"");
	}
}

} /* namespace config */
} /* namespace jerry */
