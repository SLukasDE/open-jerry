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

#include <jerry/config/Certificate.h>

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
}

Certificate::Certificate(const tinyxml2::XMLElement& element) {
	bool hasKey = false;
	bool hasCert = false;

	if(element.GetUserData() != nullptr) {
		throw esl::addStacktrace(std::runtime_error("Element has user data but it should be empty (line " + std::to_string(element.GetLineNum()) + ")"));
	}

	for(const tinyxml2::XMLAttribute* attribute = element.FirstAttribute(); attribute != nullptr; attribute = attribute->Next()) {
		if(std::string(attribute->Name()) == "domain") {
			domain = attribute->Value();
		}
		else if(std::string(attribute->Name()) == "key") {
			hasKey = true;
			keyFile = attribute->Value();
		}
		else if(std::string(attribute->Name()) == "cert") {
			hasCert = true;
			certFile = attribute->Value();
		}
		else {
			throw esl::addStacktrace(std::runtime_error(std::string("Unknown attribute \"") + attribute->Name() + "\" at line " + std::to_string(element.GetLineNum())));
		}
	}

	if(hasKey == false) {
		throw esl::addStacktrace(std::runtime_error("Missing attribute \"key\" at line " + std::to_string(element.GetLineNum())));
	}
	if(hasCert == false) {
		throw esl::addStacktrace(std::runtime_error("Missing attribute \"cert\" at line " + std::to_string(element.GetLineNum())));
	}
}

void Certificate::save(std::ostream& oStream, std::size_t spaces) const {
	oStream << makeSpaces(spaces) << "<certificate domain=\"" << domain << "\" key=\"" << keyFile << "\" cert=\"" << certFile << "\"/>\n";
}

} /* namespace config */
} /* namespace jerry */
