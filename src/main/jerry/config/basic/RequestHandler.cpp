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

#include <jerry/config/basic/RequestHandler.h>

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

RequestHandler::RequestHandler(const tinyxml2::XMLElement& element) {
	bool hasImplementation = false;

	if(element.GetUserData() != nullptr) {
		throw esl::addStacktrace(std::runtime_error("Element has user data but it should be empty (line " + std::to_string(element.GetLineNum()) + ")"));
	}

	for(const tinyxml2::XMLAttribute* attribute = element.FirstAttribute(); attribute != nullptr; attribute = attribute->Next()) {
		if(std::string(attribute->Name()) == "implementation") {
			hasImplementation = true;
			implementation = attribute->Value();
		}
		else if(std::string(attribute->Name()) == "object-implementation") {
			objectImplementation.reset(new std::string(attribute->Value()));
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

		if(innerElementName == "parameter") {
			settings.push_back(Setting(*innerElement));
		}
		else {
			throw esl::addStacktrace(std::runtime_error("Unknown element name \"" + std::string(innerElement->Name()) + "\" at line " + std::to_string(innerElement->GetLineNum())));
		}
	}

	if(hasImplementation == false) {
		throw esl::addStacktrace(std::runtime_error(std::string("Missing attribute \"implementation\" at line ") + std::to_string(element.GetLineNum())));
	}
}

void RequestHandler::save(std::ostream& oStream, std::size_t spaces) const {
	oStream << makeSpaces(spaces) << "<requesthandler implementation=\"" << implementation << "\"";

	if(objectImplementation) {
		oStream << " object-implementation=\"" << *objectImplementation << "\"";
	}

	if(settings.empty()) {
		oStream << "/>\n";
	}
	else {
		oStream << ">\n";

		for(const auto& entry : settings) {
			entry.saveParameter(oStream, spaces+2);
		}

		oStream << makeSpaces(spaces) << "</requesthandler>\n";
	}
}

} /* namespace basic */
} /* namespace config */
} /* namespace jerry */
