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

#include <jerry/config/LevelSetting.h>

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

LevelSetting::LevelSetting(const tinyxml2::XMLElement& element) {
	bool hasClass = false;
	bool hasLevel = false;

	if(element.GetUserData() != nullptr) {
		throw esl::addStacktrace(std::runtime_error("Element has user data but it should be empty (line " + std::to_string(element.GetLineNum()) + ")"));
	}

	for(const tinyxml2::XMLAttribute* attribute = element.FirstAttribute(); attribute != nullptr; attribute = attribute->Next()) {
		if(std::string(attribute->Name()) == "class") {
			hasClass = true;
			className = attribute->Value();
		}
		else if(std::string(attribute->Name()) == "level") {
			hasLevel = true;
			level = attribute->Value();
		}
		else {
			throw esl::addStacktrace(std::runtime_error(std::string("Unknown attribute \"") + attribute->Name() + "\" at line " + std::to_string(element.GetLineNum())));
		}
	}

	if(hasClass == false) {
		throw esl::addStacktrace(std::runtime_error(std::string("Missing attribute \"class\" at line ") + std::to_string(element.GetLineNum())));
	}
	if(hasLevel == false) {
		throw esl::addStacktrace(std::runtime_error(std::string("Missing attribute \"level\" at line ") + std::to_string(element.GetLineNum())));
	}
}

void LevelSetting::save(std::ostream& oStream, std::size_t spaces) const {
	oStream << makeSpaces(spaces) << "<setting class=\"" << className << "\" level=\"" << level << "\"/>\n";
}

} /* namespace config */
} /* namespace jerry */
