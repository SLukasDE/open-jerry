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

#include <jerry/config/Procedure.h>
#include <jerry/config/XMLException.h>

#include <esl/processing/procedure/Interface.h>
#include <esl/module/Interface.h>

namespace jerry {
namespace config {

Procedure::Procedure(const std::string& fileName, const tinyxml2::XMLElement& element, EngineMode engineMode, bool& hasAnonymousProcedure)
: Config(fileName, element)
{
	if(element.GetUserData() != nullptr) {
		throw XMLException(*this, "Element has user data but it should be empty");
	}

	for(const tinyxml2::XMLAttribute* attribute = element.FirstAttribute(); attribute != nullptr; attribute = attribute->Next()) {
		if(std::string(attribute->Name()) == "id") {
			id = attribute->Value();
			switch(engineMode) {
			case EngineMode::isBatch:
				if(id == "") {
					if(hasAnonymousProcedure) {
						throw XMLException(*this, "Multiple definitions of anonymous procedures are not allowed.");
					}
					hasAnonymousProcedure = true;
				}
				break;
			case EngineMode::isServer:
				if(id == "") {
					throw XMLException(*this, "Value \"\" of attribute 'id' is invalid");
				}
				break;
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

	if(id == "" && engineMode == EngineMode::isServer) {
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

void Procedure::save(std::ostream& oStream, std::size_t spaces) const {
	oStream << makeSpaces(spaces) << "<procedure id=\"" << id << "\" implementation=\"" << implementation << "\">\n";

	for(const auto& entry : settings) {
		entry.saveParameter(oStream, spaces+2);
	}
	oStream << makeSpaces(spaces) << "</procedure>\n";
}

void Procedure::install(engine::ObjectContext& engineObjectContext) const {
	engineObjectContext.addObject(id, install());
}

void Procedure::install(engine::Application& engineApplication) const {
	std::unique_ptr<esl::object::Interface::Object> eslObject = install();
	engineApplication.getLocalObjectContext().addReference(id, *eslObject);
	engineApplication.addObject(id, std::move(eslObject));
}

std::unique_ptr<esl::object::Interface::Object> Procedure::install() const {
	esl::module::Interface::Settings eslSettings;
	for(const auto& setting : settings) {
		eslSettings.push_back(std::make_pair(setting.key, evaluate(setting.value, setting.language)));
	}

	std::unique_ptr<esl::processing::procedure::Interface::Procedure> procedure;
	try {
		procedure = esl::getModule().getInterface<esl::processing::procedure::Interface>(implementation).createProcedure(eslSettings);
	}
	catch(const std::exception& e) {
		throw XMLException(*this, e.what());
	}
	catch(...) {
		throw XMLException(*this, "Could not create procedure with id '" + id + "' for implementation '" + implementation + "' because an unknown exception occurred.");
	}

	if(!procedure) {
		throw XMLException(*this, "Could not create procedure with id '" + id + "' for implementation '" + implementation + "' because interface method createProcedure() returns nullptr.");
	}

	return std::unique_ptr<esl::object::Interface::Object>(procedure.release());
}

void Procedure::parseInnerElement(const tinyxml2::XMLElement& element) {
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
