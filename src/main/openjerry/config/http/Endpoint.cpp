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

#include <openjerry/config/http/Endpoint.h>
#include <openjerry/config/http/Context.h>
#include <openjerry/config/http/RequestHandler.h>
#include <openjerry/config/http/EntryImpl.h>
#include <openjerry/config/Object.h>
#include <openjerry/config/FilePosition.h>
#include <openjerry/engine/http/Endpoint.h>

#include <esl/utility/String.h>

namespace openjerry {
namespace config {
namespace http {

Endpoint::Endpoint(const std::string& fileName, const tinyxml2::XMLElement& element)
: Config(fileName, element)
{
	if(element.GetUserData() != nullptr) {
		throw FilePosition::add(*this, "Element has user data but it should be empty");
	}

	bool hasInherit = false;

	for(const tinyxml2::XMLAttribute* attribute = element.FirstAttribute(); attribute != nullptr; attribute = attribute->Next()) {
		if(std::string(attribute->Name()) == "path") {
			path = attribute->Value();
			if(path == "") {
				throw FilePosition::add(*this, "Invalid value \"\" for attribute 'path'");
			}
		}
		else if(std::string(attribute->Name()) == "inherit") {
			std::string inheritStr = esl::utility::String::toLower(attribute->Value());
			if(hasInherit) {
				throw FilePosition::add(*this, "Multiple definition of attribute 'inherit'");
			}
			hasInherit = true;
			if(inheritStr == "true") {
				inherit = true;
			}
			else if(inheritStr == "false") {
				inherit = false;
			}
			else {
				throw FilePosition::add(*this, "Invalid value \"" + std::string(attribute->Value()) + "\" for attribute 'inherit'");
			}
		}
		else {
			throw FilePosition::add(*this, "Unknown attribute '" + std::string(attribute->Name()) + "'");
		}
	}

	if(path == "") {
		throw FilePosition::add(*this, "Missing attribute 'path'");
	}

	for(const tinyxml2::XMLNode* node = element.FirstChild(); node != nullptr; node = node->NextSibling()) {
		const tinyxml2::XMLElement* innerElement = node->ToElement();

		if(innerElement == nullptr) {
			continue;
		}

		auto oldXmlFile = setXMLFile(getFileName(), *innerElement);
		parseInnerElement(*innerElement);
		setXMLFile(oldXmlFile);
	}
}

void Endpoint::save(std::ostream& oStream, std::size_t spaces) const {
	oStream << makeSpaces(spaces) << "<endpoint path=\"" << path << "\">\n";

	for(const auto& entry : entries) {
		entry->save(oStream, spaces+2);
	}

	for(const auto& responseHeader : responseHeaders) {
		responseHeader.saveResponseHeader(oStream, spaces+2);
	}

	exceptions.save(oStream, spaces+2);

	oStream << makeSpaces(spaces) << "<endpoint/>\n";
}

void Endpoint::install(engine::http::Context& engineHttpContext) const {
	std::unique_ptr<engine::http::Endpoint> httpEndpoint(new engine::http::Endpoint(engineHttpContext.getProcessRegistry(), path));
	engine::http::Endpoint& httpEndpointRef = *httpEndpoint;

	if(inherit) {
		httpEndpointRef.setParent(&engineHttpContext);
	}

	engineHttpContext.addEndpoint(std::move(httpEndpoint));


	/* *****************
	 * install entries *
	 * *****************/
	for(const auto& entry : entries) {
		entry->install(httpEndpointRef);
	}

	/* **********************
	 * Set response headers *
	 * **********************/
	for(const auto& responseHeader : responseHeaders) {
		httpEndpointRef.addHeader(responseHeader.key, responseHeader.value);
	}

	exceptions.install(httpEndpointRef);
}

void Endpoint::parseInnerElement(const tinyxml2::XMLElement& element) {
	if(element.Name() == nullptr) {
		throw FilePosition::add(*this, "Element name is empty");
	}

	std::string innerElementName(element.Name());

	if(innerElementName == "response-header") {
		responseHeaders.push_back(Setting(getFileName(), element, false));
	}
	else if(innerElementName == "exceptions") {
		exceptions = Exceptions(getFileName(), element);
	}
	else {
		entries.emplace_back(new EntryImpl(getFileName(), element));
	}
}

} /* namespace http */
} /* namespace config */
} /* namespace openjerry */
