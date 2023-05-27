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

#include <openjerry/config/ProcedureContext.h>
#include <openjerry/config/procedure/EntryImpl.h>
#include <openjerry/config/Object.h>
#include <openjerry/config/FilePosition.h>

//#include <esl/stacktrace/Stacktrace.h>
#include <esl/utility/String.h>

#include <stdexcept>

namespace openjerry {
namespace config {

ProcedureContext::ProcedureContext(const std::string& fileName, const tinyxml2::XMLElement& element)
: Config(fileName, element)
{
	if(element.GetUserData() != nullptr) {
		throw FilePosition::add(*this, "Element has user data but it should be empty");
	}

	bool hasInherit = false;

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
			if(hasInherit) {
				throw FilePosition::add(*this, "Attribute 'ref-id' is not allowed together with attribute 'inherit'.");
			}
		}
		else if(std::string(attribute->Name()) == "inherit") {
			if(hasInherit) {
				throw FilePosition::add(*this, "Multiple definition of attribute 'inherit'");
			}
			std::string inheritStr = esl::utility::String::toLower(attribute->Value());
			hasInherit = true;
			if(inheritStr == "true") {
				inherit = true;
			}
			else if(inheritStr == "false") {
				inherit = false;
			}
			else {
				throw FilePosition::add(*this, "Invalid value \"" + std::string(attribute->Value()) + "\" for attribute 'inherit'");
			}
			if(refId != "") {
				throw FilePosition::add(*this, "Attribute 'inherit' is not allowed together with attribute 'ref-id'.");
			}
		}
		else {
			throw FilePosition::add(*this, "Unknown attribute '" + std::string(attribute->Name()) + "'");
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

const std::string& ProcedureContext::getId() const noexcept {
	return id;
}

const std::string& ProcedureContext::getRefId() const noexcept {
	return refId;
}

bool ProcedureContext::getInherit() const noexcept {
	return inherit;
}

void ProcedureContext::installEntries(engine::procedure::Context& newContext) const {
	/* *****************
	 * install entries *
	 * *****************/
	for(const auto& entry : entries) {
		entry->install(newContext);
	}
}

void ProcedureContext::saveEntries(std::ostream& oStream, std::size_t spaces) const {
	for(const auto& entry : entries) {
		entry->save(oStream, spaces+2);
	}
}

void ProcedureContext::parseInnerElement(const tinyxml2::XMLElement& element) {
	entries.emplace_back(new procedure::EntryImpl(getFileName(), element));
}

} /* namespace config */
} /* namespace openjerry */
