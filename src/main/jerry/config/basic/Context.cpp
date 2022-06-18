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

#include <jerry/config/basic/Context.h>
#include <jerry/config/basic/EntryImpl.h>
#include <jerry/config/basic/RequestHandler.h>
#include <jerry/config/Object.h>
#include <jerry/config/XMLException.h>

#include <esl/stacktrace/Stacktrace.h>
#include <esl/utility/String.h>

#include <stdexcept>

namespace jerry {
namespace config {
namespace basic {

Context::Context(const std::string& fileName, const tinyxml2::XMLElement& element)
: Config(fileName, element)
{
	if(element.GetUserData() != nullptr) {
		throw XMLException(*this, "Element has user data but it should be empty");
	}

	bool hasInherit = false;

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
			if(hasInherit) {
				throw XMLException(*this, "Attribute 'ref-id' is not allowed together with attribute 'inherit'.");
			}
		}
		else if(std::string(attribute->Name()) == "inherit") {
			if(hasInherit) {
				throw XMLException(*this, "Multiple definition of attribute 'inherit'");
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
				throw XMLException(*this, "Invalid value \"" + std::string(attribute->Value()) + "\" for attribute 'inherit'");
			}
			if(refId != "") {
				throw XMLException(*this, "Attribute 'inherit' is not allowed together with attribute 'ref-id'.");
			}
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

void Context::save(std::ostream& oStream, std::size_t spaces) const {
	oStream << makeSpaces(spaces) << "<context";

	if(refId != "") {
		oStream << " ref-id=\"" << refId << "\"/>\n";
	}
	else {
		if(id != "") {
			oStream << " id=\"" << id << "\"";
		}
		if(inherit) {
			oStream << " inherit=\"true\"";
		}
		else {
			oStream << " inherit=\"false\"";
		}
		oStream << ">\n";

		for(const auto& entry : entries) {
			entry->save(oStream, spaces+2);
		}

		oStream << makeSpaces(spaces) << "</context>\n";
	}
}

void Context::install(engine::basic::Context& engineBasicContext) const {
	if(refId.empty()) {
		std::unique_ptr<engine::basic::Context> basicContext(new engine::basic::Context(engineBasicContext.getProcessRegistry()));
		engine::basic::Context& basicContextRef = *basicContext;

		if(inherit) {
			basicContextRef.setParent(&engineBasicContext);
		}

		if(id.empty()) {
			engineBasicContext.addContext(std::move(basicContext));
		}
		else {
			engineBasicContext.addObject(id, std::unique_ptr<esl::object::Interface::Object>(basicContext.release()));
		}

		/* *****************
		 * install entries *
		 * *****************/
		installEntries(basicContextRef);
	}
	else {
		engineBasicContext.addContext(refId);
	}
}

const std::string& Context::getId() const noexcept {
	return id;
}

const std::string& Context::getRefId() const noexcept {
	return refId;
}

bool Context::getInherit() const noexcept {
	return inherit;
}

void Context::installEntries(engine::basic::Context& newContext) const {
	/* *****************
	 * install entries *
	 * *****************/
	for(const auto& entry : entries) {
		entry->install(newContext);
	}
}

void Context::parseInnerElement(const tinyxml2::XMLElement& element) {
	if(!refId.empty()) {
		throw XMLException(*this, "No content allowed if 'ref-id' is specified.");
	}

	entries.emplace_back(new EntryImpl(getFileName(), element));
}

} /* namespace basic */
} /* namespace config */
} /* namespace jerry */
