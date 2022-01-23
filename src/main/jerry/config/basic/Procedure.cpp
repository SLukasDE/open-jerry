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

#include <jerry/config/basic/Procedure.h>
#include <jerry/config/XMLException.h>

#include <esl/processing/procedure/Interface.h>
#include <esl/utility/String.h>

namespace jerry {
namespace config {
namespace basic {

Procedure::Procedure(const std::string& fileName, const tinyxml2::XMLElement& element)
: Config(fileName, element)
{
	if(element.GetUserData() != nullptr) {
		throw XMLException(*this, "Element has user data but it should be empty");
	}

	for(const tinyxml2::XMLAttribute* attribute = element.FirstAttribute(); attribute != nullptr; attribute = attribute->Next()) {
		if(std::string(attribute->Name()) == "id") {
			if(id != "") {
				throw XMLException(*this, "Multiple definition of attribute 'id'");
			}
			id = attribute->Value();
			if(id == "") {
				throw XMLException(*this, "Invalid value \"\" for attribute 'id'");
			}
			if(refId != "") {
				throw XMLException(*this, "Attribute 'id' is not allowed together with attribute 'ref-id'.");
			}
		}
		else if(std::string(attribute->Name()) == "implementation") {
			implementation = attribute->Value();
			if(implementation == "") {
				throw XMLException(*this, "Value \"\" of attribute 'implementation' is invalid");
			}
			if(refId != "") {
				throw XMLException(*this, "Attribute 'implementation' is not allowed together with attribute 'ref-id'.");
			}
		}
		else if(std::string(attribute->Name()) == "ref-id") {
			if(refId != "") {
				throw XMLException(*this, "Multiple definition of attribute 'ref-id'");
			}
			refId = attribute->Value();
			if(refId == "") {
				throw XMLException(*this, "Invalid value \"\" for attribute 'ref-id'");
			}
			if(id != "") {
				throw XMLException(*this, "Attribute 'ref-id' is not allowed together with attribute 'id'.");
			}
			if(implementation != "") {
				throw XMLException(*this, "Attribute 'ref-id' is not allowed together with attribute 'implementation'.");
			}
		}
		else {
			throw XMLException(*this, "Unknown attribute '" + std::string(attribute->Name()) + "'");
		}
	}

	if(refId == "" && implementation == "") {
		throw XMLException(*this, "Attribute 'implementation' is missing.");
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
	oStream << makeSpaces(spaces) << "<procedure";

	if(refId != "") {
		oStream << " ref-id=\"" << refId << "\"/>\n";
	}
	else {
		if(id != "") {
			oStream << " id=\"" << id << "\"";
		}
		if(implementation != "") {
			oStream << " implementation=\"" << implementation << "\"";
		}
		oStream << ">\n";

		for(const auto& setting : settings) {
			setting.saveParameter(oStream, spaces+2);
		}

		oStream << makeSpaces(spaces) << "</procedure>\n";
	}
}

void Procedure::install(engine::basic::Context& engineBasicContext) const {
	if(refId == "") {
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
			throw XMLException(*this, "Could not create a procedure with id '" + id + "' for implementation '" + implementation + "' because an unknown exception occurred.");
		}

		if(!procedure) {
			throw XMLException(*this, "Could not create a procedure with id '" + id + "' for implementation '" + implementation + "' because interface method createProcedure() returns nullptr.");
		}

		if(id != "") {
			engineBasicContext.addObject(id, std::unique_ptr<esl::object::Interface::Object>(procedure.release()));
		}
		else {
			engineBasicContext.addProcedure(std::move(procedure));
		}
	}
	else {
		engineBasicContext.addProcedure(refId);
	}
}

void Procedure::parseInnerElement(const tinyxml2::XMLElement& element) {
	if(refId != "") {
		throw XMLException(*this, "No content allowed if 'ref-id' is specified.");
	}

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

} /* namespace basic */
} /* namespace config */
} /* namespace jerry */
