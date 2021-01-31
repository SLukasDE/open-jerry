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

#include <jerry/config/http/Server.h>

#include <esl/utility/String.h>
#include <esl/Stacktrace.h>

#include <stdexcept>

namespace jerry {
namespace config {
namespace http {

namespace {
std::string makeSpaces(std::size_t spaces) {
	std::string rv;
	for(std::size_t i=0; i<spaces; ++i) {
		rv += " ";
	}
	return rv;
}
}

Server::Server(const tinyxml2::XMLElement& element) {
	bool hasPort = false;

	if(element.GetUserData() != nullptr) {
		throw esl::addStacktrace(std::runtime_error("Element has user data but it should be empty (line " + std::to_string(element.GetLineNum()) + ")"));
	}

	for(const tinyxml2::XMLAttribute* attribute = element.FirstAttribute(); attribute != nullptr; attribute = attribute->Next()) {
		// 	<http-server id="http-1" implementation="mhd4esl" port="8080" https="true">
		if(std::string(attribute->Name()) == "id") {
			id = attribute->Value();
		}
		else if(std::string(attribute->Name()) == "implementation") {
			implementation = attribute->Value();
		}
		else if(std::string(attribute->Name()) == "port") {
			port = std::stoi(std::string(attribute->Value()));
			hasPort = true;
		}
		else if(std::string(attribute->Name()) == "https") {
			std::string httpsStr = esl::utility::String::toLower(attribute->Value());
			if(httpsStr == "true") {
				isHttps = true;
			}
			else if(httpsStr == "false") {
				isHttps = false;
			}
			else {
				throw esl::addStacktrace(std::runtime_error(std::string("Invalid value \"") + attribute->Value() + "\" for attribute \"https\" at line " + std::to_string(element.GetLineNum())));
			}
		}
		else {
			throw esl::addStacktrace(std::runtime_error(std::string("Unknown attribute \"") + attribute->Name() + "\" at line " + std::to_string(element.GetLineNum())));
		}
	}

	if(hasPort == false) {
		throw esl::addStacktrace(std::runtime_error(std::string("Missing attribute \"port\" at line ") + std::to_string(element.GetLineNum())));
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
}

void Server::save(std::ostream& oStream, std::size_t spaces) const {
	// 	<http-server id="http-1" implementation="mhd4esl" port="8080" https="true">
	oStream << makeSpaces(spaces) << "<http-server id=\"" << id << "\" implementation=\"" << implementation << "\" port=\"" << port << "\" https=\"";
	oStream << (isHttps ? "true" : "false");
	oStream << "\">\n";

	for(const auto& entry : settings) {
		entry.saveParameter(oStream, spaces+2);
	}

	oStream << makeSpaces(spaces) << "<http-server/>\n";
}

} /* namespace http */
} /* namespace config */
} /* namespace jerry */
