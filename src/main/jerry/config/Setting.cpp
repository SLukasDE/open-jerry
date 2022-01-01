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

#include <jerry/config/Setting.h>
#include <jerry/config/XMLException.h>

namespace jerry {
namespace config {

Setting::Setting(const std::string& fileName, const tinyxml2::XMLElement& element, bool isParameter)
: Config(fileName, element)
{
	if(element.GetUserData() != nullptr) {
		throw XMLException(*this, "Element has user data but it should be empty");
	}

	for(const tinyxml2::XMLAttribute* attribute = element.FirstAttribute(); attribute != nullptr; attribute = attribute->Next()) {
		if(std::string(attribute->Name()) == "key") {
			key = attribute->Value();
			if(key == "") {
				throw XMLException(*this, "Value \"\" of attribute 'key' is invalid.");
			}
		}
		else if(std::string(attribute->Name()) == "value") {
			value = attribute->Value();
		}
		else if(std::string(attribute->Name()) == "language" && isParameter) {
			language = attribute->Value();
		}
		else {
			throw XMLException(*this, "Unknown attribute '" + std::string(attribute->Name()) + "'");
		}
	}

	if(key == "") {
		throw XMLException(*this, "Missing attribute 'key'");
	}
}

void Setting::saveParameter(std::ostream& oStream, std::size_t spaces) const {
	oStream << makeSpaces(spaces) << "<parameter key=\"" << key << "\" value=\"" << value << "\" language=\"" << language << "\"/>\n";
}

void Setting::saveLayout(std::ostream& oStream, std::size_t spaces) const {
	oStream << makeSpaces(spaces) << "<layout key=\"" << key << "\" value=\"" << value << "\"/>\n";
}

void Setting::saveResponseHeader(std::ostream& oStream, std::size_t spaces) const {
	oStream << makeSpaces(spaces) << "<response-header key=\"" << key << "\" value=\"" << value << "\"/>\n";
}

} /* namespace confing */
} /* namespace jerry */
