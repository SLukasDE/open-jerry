/*
 * This file is part of Jerry application server.
 * Copyright (C) 2020 Sven Lukas
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

#include <jerry/config/ExceptionDocument.h>
#include <jerry/utility/URL.h>

#include <esl/Stacktrace.h>

#include <stdexcept>

namespace jerry {
namespace config {

namespace {
std::string makeSpaces(std::size_t spaces) {
	std::string rv;
	for(std::size_t i=0; i<spaces; ++i) {
		rv += " ";
	}
	return rv;
}

bool stringToBool(bool& b, std::string str) {
	if(str == "true") {
		b = true;
	}
	else if(str == "false") {
		b = false;
	}
	else {
		return false;
	}
	return true;
}
}

ExceptionDocument::ExceptionDocument(const tinyxml2::XMLElement& element) {
	bool hasStatusCode = false;
	bool hasPath = false;

	if(element.GetUserData() != nullptr) {
		throw esl::addStacktrace(std::runtime_error("Element has user data but it should be empty (line " + std::to_string(element.GetLineNum()) + ")"));
	}

	for(const tinyxml2::XMLAttribute* attribute = element.FirstAttribute(); attribute != nullptr; attribute = attribute->Next()) {
		if(std::string(attribute->Name()) == "statusCode") {
			hasStatusCode = true;
			statusCode= std::atoi(attribute->Value());
		}
		else if(std::string(attribute->Name()) == "path") {
			hasPath = true;
			path = attribute->Value();
		}
		else if(std::string(attribute->Name()) == "parser") {
			if(stringToBool(parser, attribute->Value()) == false) {
				throw esl::addStacktrace(std::runtime_error(std::string("Unknown value \"") + attribute->Value() + "\" for attribute \"" + attribute->Name() + "\" at line " + std::to_string(element.GetLineNum())));
			}
		}
		else {
			throw esl::addStacktrace(std::runtime_error(std::string("Unknown attribute \"") + attribute->Name() + "\" at line " + std::to_string(element.GetLineNum())));
		}
	}

	if(hasStatusCode == false) {
		throw esl::addStacktrace(std::runtime_error(std::string("Missing attribute \"statusCode\" at line ") + std::to_string(element.GetLineNum())));
	}
	if(hasPath == false) {
		throw esl::addStacktrace(std::runtime_error(std::string("Missing attribute \"path\" at line ") + std::to_string(element.GetLineNum())));
	}

	if(parser) {
		utility::URL url(path);
		if(!url.getScheme().empty() && url.getScheme() != "file") {
			throw esl::addStacktrace(std::runtime_error(std::string("Attribute parse=\"true\" at line ") + std::to_string(element.GetLineNum()) + " is only allowed with file protocol specified for attribute \"path\"."));
		}
	}
}

void ExceptionDocument::save(std::ostream& oStream, std::size_t spaces) const {
	oStream << makeSpaces(spaces) << "<document statusCode=\"" << statusCode << "\" path=\"" << path << "\" parser=\"";
	if(parser) {
		oStream << "true";
	}
	else {
		oStream << "false";
	}
	oStream << "\"/>\n";
}

} /* namespace config */
} /* namespace jerry */
