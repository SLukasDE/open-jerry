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

#include <jerry/config/basic/BasicListener.h>
#include <jerry/config/basic/EntryImpl.h>
#include <jerry/config/XMLException.h>
#include <jerry/engine/basic/Context.h>

#include <esl/Stacktrace.h>
#include <esl/utility/String.h>

#include <stdexcept>

namespace jerry {
namespace config {
namespace basic {

BasicListener::BasicListener(const std::string& fileName, const tinyxml2::XMLElement& element)
: Config(fileName, element)
{
	if(element.GetUserData() != nullptr) {
		throw jerry::config::XMLException(*this, "Element has user data but it should be empty");
	}

	bool hasInherit = false;

	for(const tinyxml2::XMLAttribute* attribute = element.FirstAttribute(); attribute != nullptr; attribute = attribute->Next()) {
		if(std::string(attribute->Name()) == "inherit") {
			if(hasInherit) {
				throw jerry::config::XMLException(*this, "Multiple definition of attribute 'inherit'");
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
				throw jerry::config::XMLException(*this, "Invalid value \"" + std::string(attribute->Value()) + "\" for attribute 'inherit'");
			}
		}
		else {
			throw jerry::config::XMLException(*this, "Unknown attribute '" + std::string(attribute->Name()) + "'");
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

void BasicListener::save(std::ostream& oStream, std::size_t spaces) const {
	oStream << makeSpaces(spaces) << "<basic-listener";
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

	oStream << makeSpaces(spaces) << "</basic-listener>\n";
}

void BasicListener::install(engine::Application& engineApplication) const {
	engine::basic::Context& engineContext = engineApplication.addBasicListener();
	if(inherit) {
		engineContext.setParent(&engineApplication);
	}
	else {
		engineContext.setParent(&engineApplication.getLocalObjectContext());
	}


	/* *****************
	 * install entries *
	 * *****************/
	for(const auto& entry : entries) {
		entry->install(engineContext);
	}
}

void BasicListener::parseInnerElement(const tinyxml2::XMLElement& element) {
	entries.emplace_back(new jerry::config::basic::EntryImpl(getFileName(), element));
}

} /* namespace basic */
} /* namespace config */
} /* namespace jerry */
