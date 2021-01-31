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

#include <jerry/config/messaging/Context.h>
#include <jerry/config/messaging/Endpoint.h>
#include <jerry/config/messaging/MessageHandler.h>
#include <jerry/config/Object.h>
#include <jerry/config/Reference.h>

#include <esl/Stacktrace.h>

#include <stdexcept>

namespace jerry {
namespace config {
namespace messaging {

namespace {
std::string makeSpaces(std::size_t spaces) {
	std::string rv;
	for(std::size_t i=0; i<spaces; ++i) {
		rv += " ";
	}
	return rv;
}
}

Context::Context(const tinyxml2::XMLElement& element, bool aHasEndpoint, bool aIsGlobal)
: hasEndpoint(aHasEndpoint),
  isGlobal(aIsGlobal)
{
	if(element.GetUserData() != nullptr) {
		throw esl::addStacktrace(std::runtime_error("Element has user data but it should be empty (line " + std::to_string(element.GetLineNum()) + ")"));
	}

	for(const tinyxml2::XMLAttribute* attribute = element.FirstAttribute(); attribute != nullptr; attribute = attribute->Next()) {
		if(std::string(attribute->Name()) == "id" && isGlobal == true) {
			hasId = true;
			id = attribute->Value();
		}
		else if(std::string(attribute->Name()) == "ref-id" && isGlobal == false) {
			hasRefId = true;
			refId = attribute->Value();
		}
		else {
			throw esl::addStacktrace(std::runtime_error(std::string("Unknown attribute \"") + attribute->Name() + "\" at line " + std::to_string(element.GetLineNum())));
		}
	}

	if(isGlobal && hasId == false) {
		throw esl::addStacktrace(std::runtime_error("Attribute \"id\" is missing at line " + std::to_string(element.GetLineNum())));
	}

	for(const tinyxml2::XMLNode* node = element.FirstChild(); node != nullptr; node = node->NextSibling()) {
		const tinyxml2::XMLElement* innerElement = node->ToElement();

		if(innerElement == nullptr) {
			continue;
		}

		entries.push_back(Entry(*innerElement, hasEndpoint));
	}
}

void Context::save(std::ostream& oStream, std::size_t spaces) const {
	if(isGlobal) {
		oStream << makeSpaces(spaces) << "<message-context";
	}
	else {
		oStream << makeSpaces(spaces) << "<context";
	}
	if(hasId) {
		oStream << makeSpaces(spaces) << "id=\"" << id << "\"";
	}
	if(hasRefId) {
		oStream << makeSpaces(spaces) << "ref-id=\"" << refId << "\"";
	}
	oStream << makeSpaces(spaces) << ">\n";

	for(const auto& entry : entries) {
		entry.save(oStream, spaces+2);
	}

	if(isGlobal) {
		oStream << makeSpaces(spaces) << "<message-context/>\n";
	}
	else {
		oStream << makeSpaces(spaces) << "<context/>\n";
	}
}

} /* namespace messaging */
} /* namespace config */
} /* namespace jerry */
