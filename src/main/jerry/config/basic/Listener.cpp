/*
 * This file is part of Jerry application server.
 * Copyright (C) 2020-2021 Sven Lukas
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

#include <jerry/config/basic/Listener.h>
#include <jerry/config/basic/Context.h>

#include <esl/Stacktrace.h>

#include <stdexcept>

namespace jerry {
namespace config {
namespace basic {

namespace {
std::string makeSpaces(std::size_t spaces) {
	std::string rv;
	for(std::size_t i=0; i<spaces; ++i) {
		rv += " ";
	}
	return rv;
}
}

Listener::Listener(const tinyxml2::XMLElement& element) {
	if(element.GetUserData() != nullptr) {
		throw esl::addStacktrace(std::runtime_error("Element has user data but it should be empty (line " + std::to_string(element.GetLineNum()) + ")"));
	}

	for(const tinyxml2::XMLAttribute* attribute = element.FirstAttribute(); attribute != nullptr; attribute = attribute->Next()) {
		if(std::string(attribute->Name()) == "ref-id") {
			refId = attribute->Value();
		}
		else {
			throw esl::addStacktrace(std::runtime_error(std::string("Unknown attribute \"") + attribute->Name() + "\" at line " + std::to_string(element.GetLineNum())));
		}
	}

	for(const tinyxml2::XMLNode* node = element.FirstChild(); node != nullptr; node = node->NextSibling()) {
		const tinyxml2::XMLElement* innerElement = node->ToElement();

		if(innerElement == nullptr) {
			continue;
		}

		if(innerElement->Name() == nullptr) {
			throw esl::addStacktrace(std::runtime_error("Element name is empty at line " + std::to_string(innerElement->GetLineNum())));
		}

		std::string innerElementName(innerElement->Name());

		/*
		if(innerElementName == "exceptions") {
			exceptions = Exceptions(*innerElement);
		}
		else {
			entries.push_back(Entry(*innerElement));
		}
		*/
		entries.push_back(Entry(*innerElement));
	}
}

void Listener::save(std::ostream& oStream, std::size_t spaces) const {
	// <basic-listener ref-id="broker-1">
	oStream << makeSpaces(spaces) << "<basic-listener ref-id=\"" << refId << "\">\n";

	for(const auto& entry : entries) {
		entry.save(oStream, spaces+2);
	}

	/*
	for(const auto& entry : responseHeaders) {
		entry.saveResponseHeader(oStream, spaces+2);
	}

	exceptions.save(oStream, spaces+2);
	*/

	oStream << makeSpaces(spaces) << "<basic-listener/>\n";
}

} /* namespace basic */
} /* namespace config */
} /* namespace jerry */
