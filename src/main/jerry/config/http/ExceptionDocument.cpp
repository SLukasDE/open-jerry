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

#include <jerry/config/http/ExceptionDocument.h>
#include <jerry/config/XMLException.h>

#include <esl/utility/URL.h>
#include <esl/utility/Protocol.h>
#include <esl/stacktrace/Stacktrace.h>

#include <stdexcept>

namespace jerry {
namespace config {
namespace http {

ExceptionDocument::ExceptionDocument(const std::string& fileName, const tinyxml2::XMLElement& element)
: Config(fileName, element)
{
	bool hasStatusCode = false;
	bool hasPath = false;

	if(element.GetUserData() != nullptr) {
		throw XMLException(*this, "Element has user data but it should be empty");
	}

	for(const tinyxml2::XMLAttribute* attribute = element.FirstAttribute(); attribute != nullptr; attribute = attribute->Next()) {
		std::string attributeName(attribute->Name());

		if(attributeName == "statusCode") {
			hasStatusCode = true;
			statusCode= std::atoi(attribute->Value());
		}
		else if(attributeName == "path") {
			hasPath = true;
			path = attribute->Value();
		}
		else if(attributeName == "parser") {
			if(stringToBool(parser, attribute->Value()) == false) {
				throw XMLException(*this, "Unknown value \"" + std::string(attribute->Value()) + "\" for attribute '" + attributeName + "'");
			}
		}
		else {
			throw XMLException(*this, "Unknown attribute '" + attributeName + "'");
		}
	}

	if(hasStatusCode == false) {
		throw XMLException(*this, "Missing attribute 'statusCode'");
	}
	if(hasPath == false) {
		throw XMLException(*this, "Missing attribute 'path'");
	}

	if(parser) {
		esl::utility::URL url(path);
		if(url.getScheme() && url.getScheme() != esl::utility::Protocol::Type::file) {
			throw XMLException(*this, "Attribute parse=\"true\" is only allowed with file protocol specified for attribute 'path'.");
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

} /* namespace http */
} /* namespace config */
} /* namespace jerry */
