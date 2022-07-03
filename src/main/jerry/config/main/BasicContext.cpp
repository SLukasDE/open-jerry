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

#include <jerry/config/main/BasicContext.h>
#include <jerry/config/basic/RequestHandler.h>
#include <jerry/config/basic/EntryImpl.h>
#include <jerry/config/Object.h>
#include <jerry/config/XMLException.h>
#include <jerry/engine/basic/Context.h>

#include <esl/utility/String.h>

#include <stdexcept>

#include <memory>

namespace jerry {
namespace config {
namespace main {

BasicContext::BasicContext(const std::string& fileName, const tinyxml2::XMLElement& element)
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
		}
		else {
			throw XMLException(*this, "Unknown attribute '" + std::string(attribute->Name()) + "'");
		}
	}

	if(id == "") {
		throw XMLException(*this, "Attribute 'id' is missing");
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

void BasicContext::save(std::ostream& oStream, std::size_t spaces) const {
	oStream << makeSpaces(spaces) << "<basic-context id=\"" << id << "\"";

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

	oStream << makeSpaces(spaces) << "</basic-context>\n";
}

void BasicContext::install(engine::ObjectContext& engineObjectContext) const {
	std::unique_ptr<engine::basic::Context> context(new engine::basic::Context(engineObjectContext.getProcessRegistry()));
	engine::basic::Context& contextRef = *context;

	engineObjectContext.addObject(getId(), std::unique_ptr<esl::object::Object>(context.release()));

	if(getInherit()) {
		contextRef.ObjectContext::setParent(&engineObjectContext);
	}

	installEntries(contextRef);
}

const std::string& BasicContext::getId() const noexcept {
	return id;
}

bool BasicContext::getInherit() const noexcept {
	return inherit;
}

void BasicContext::installEntries(engine::basic::Context& newContext) const {
	/* *****************
	 * install entries *
	 * *****************/
	for(const auto& entry : entries) {
		entry->install(newContext);
	}
}

void BasicContext::parseInnerElement(const tinyxml2::XMLElement& element) {
	entries.emplace_back(new basic::EntryImpl(getFileName(), element));
}

} /* namespace main */
} /* namespace config */
} /* namespace jerry */
