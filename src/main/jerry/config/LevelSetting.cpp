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

#include <jerry/config/LevelSetting.h>
#include <jerry/config/XMLException.h>

namespace jerry {
namespace config {

LevelSetting::LevelSetting(const std::string& fileName, const tinyxml2::XMLElement& element)
: Config(fileName, element)
{
	if(element.GetUserData() != nullptr) {
		throw XMLException(*this, "Element has user data but it should be empty");
	}

	for(const tinyxml2::XMLAttribute* attribute = element.FirstAttribute(); attribute != nullptr; attribute = attribute->Next()) {
		if(std::string(attribute->Name()) == "class") {
			className = attribute->Value();
			if(className == "") {
				throw XMLException(*this, "Value \"\" of attribute 'class' is invalid.");
			}
		}
		else if(std::string(attribute->Name()) == "level") {
			level = attribute->Value();
			if(level == "") {
				throw XMLException(*this, "Value \"\" of attribute 'level' is invalid.");
			}
		}
		else {
			throw XMLException(*this, "Unknown attribute '" + std::string(attribute->Name()) + "'");
		}
	}

	if(className == "") {
		throw XMLException(*this, "Missing attribute 'class'");
	}
	if(level == "") {
		throw XMLException(*this, "Missing attribute 'level'");
	}
}

void LevelSetting::save(std::ostream& oStream, std::size_t spaces) const {
	oStream << makeSpaces(spaces) << "<setting class=\"" << className << "\" level=\"" << level << "\"/>\n";
}

} /* namespace config */
} /* namespace jerry */
