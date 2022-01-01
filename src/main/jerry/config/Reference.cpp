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

#include <jerry/config/Reference.h>
#include <jerry/config/XMLException.h>

namespace jerry {
namespace config {

Reference::Reference(const std::string& fileName, const tinyxml2::XMLElement& element)
: Config(fileName, element)
{
	if(element.GetUserData() != nullptr) {
		throw XMLException(*this, "Element has user data but it should be empty");
	}

	for(const tinyxml2::XMLAttribute* attribute = element.FirstAttribute(); attribute != nullptr; attribute = attribute->Next()) {
		if(std::string(attribute->Name()) == "id") {
			id = attribute->Value();
			if(id == "") {
				throw XMLException(*this, "Value \"\" of attribute 'id' is invalid.");
			}
		}
		else if(std::string(attribute->Name()) == "ref-id") {
			refId = attribute->Value();
			if(id == "") {
				throw XMLException(*this, "Value \"\" of attribute 'ref-id' is invalid.");
			}
		}
		else {
			throw XMLException(*this, "Unknown attribute '" + std::string(attribute->Name()) + "'");
		}
	}

	if(id == "") {
		throw XMLException(*this, "Missing attribute 'id'");
	}
	if(refId == "") {
		throw XMLException(*this, "Missing attribute 'ref-id'");
	}
}

void Reference::save(std::ostream& oStream, std::size_t spaces) const {
	oStream << makeSpaces(spaces) << "<reference id=\"" << id << "\" ref-id=\"" << refId << "\"/>\n";
}

void Reference::install(engine::ObjectContext& engineObjectContext) const {
	engineObjectContext.addReference(id, refId);
}

} /* namespace config */
} /* namespace jerry */

