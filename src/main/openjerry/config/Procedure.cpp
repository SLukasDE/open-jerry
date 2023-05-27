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

#include <openjerry/config/Procedure.h>
#include <openjerry/config/FilePosition.h>

#include <esl/plugin/exception/PluginNotFound.h>
#include <esl/plugin/Registry.h>

namespace openjerry {
namespace config {

Procedure::Procedure(const std::string& fileName, const tinyxml2::XMLElement& element)
: Config(fileName, element)
{
	if(element.GetUserData() != nullptr) {
		throw FilePosition::add(*this, "Element has user data but it should be empty");
	}

	for(const tinyxml2::XMLAttribute* attribute = element.FirstAttribute(); attribute != nullptr; attribute = attribute->Next()) {
		if(std::string(attribute->Name()) == "id") {
			if(id != "") {
				throw FilePosition::add(*this, "Multiple definition of attribute 'id'");
			}
			id = attribute->Value();
			if(id == "") {
				throw FilePosition::add(*this, "Invalid value \"\" for attribute 'id'");
			}
			if(refId != "") {
				throw FilePosition::add(*this, "Attribute 'id' is not allowed together with attribute 'ref-id'.");
			}
		}
		else if(std::string(attribute->Name()) == "implementation") {
			implementation = attribute->Value();
			if(implementation == "") {
				throw FilePosition::add(*this, "Value \"\" of attribute 'implementation' is invalid");
			}
			if(refId != "") {
				throw FilePosition::add(*this, "Attribute 'implementation' is not allowed together with attribute 'ref-id'.");
			}
		}
		else if(std::string(attribute->Name()) == "ref-id") {
			if(refId != "") {
				throw FilePosition::add(*this, "Multiple definition of attribute 'ref-id'");
			}
			refId = attribute->Value();
			if(refId == "") {
				throw FilePosition::add(*this, "Invalid value \"\" for attribute 'ref-id'");
			}
			if(id != "") {
				throw FilePosition::add(*this, "Attribute 'ref-id' is not allowed together with attribute 'id'.");
			}
			if(implementation != "") {
				throw FilePosition::add(*this, "Attribute 'ref-id' is not allowed together with attribute 'implementation'.");
			}
		}
		else {
			throw FilePosition::add(*this, "Unknown attribute '" + std::string(attribute->Name()) + "'");
		}
	}

	if(refId == "" && implementation == "") {
		throw FilePosition::add(*this, "Attribute 'implementation' is missing.");
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

	if(refId.empty()) {
		if(!id.empty()) {
			oStream << " id=\"" << id << "\"";
		}
		if(!implementation.empty()) {
			oStream << " implementation=\"" << implementation << "\"";
		}
		oStream << ">\n";

		for(const auto& setting : settings) {
			setting.saveParameter(oStream, spaces+2);
		}

		oStream << makeSpaces(spaces) << "</procedure>\n";
	}
	else {
		oStream << " ref-id=\"" << refId << "\"/>\n";
	}
}

const std::string& Procedure::getId() const noexcept {
	return id;
}

const std::string& Procedure::getRefId() const noexcept {
	return refId;
}

std::unique_ptr<esl::processing::Procedure> Procedure::create() const {
	std::vector<std::pair<std::string, std::string>> eslSettings;
	for(const auto& setting : settings) {
		eslSettings.push_back(std::make_pair(setting.key, evaluate(setting.value, setting.language)));
	}

	std::unique_ptr<esl::processing::Procedure> procedure;
	try {
		procedure = esl::plugin::Registry::get().create<esl::processing::Procedure>(implementation, eslSettings);
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
		throw FilePosition::add(*this, "Could not create procedure with id '" + id + "' for implementation '" + implementation + "' because an unknown exception occurred.");
	}

	if(!procedure) {
		throw FilePosition::add(*this, "Could not create procedure with id '" + id + "' for implementation '" + implementation + "' because interface method createProcedure() returns nullptr.");
	}

	return procedure;
}

void Procedure::parseInnerElement(const tinyxml2::XMLElement& element) {
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
} /* namespace openjerry */
