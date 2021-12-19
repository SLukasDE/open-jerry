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

#include <jerry/config/http/Listener.h>
#include <jerry/config/http/Endpoint.h>
#include <jerry/config/http/Context.h>
#include <jerry/config/http/RequestHandler.h>
#include <jerry/config/XMLException.h>

#include <esl/utility/String.h>

namespace jerry {
namespace config {
namespace http {

Listener::Listener(const std::string& fileName, const tinyxml2::XMLElement& element)
: Config(fileName, element)
{
	if(element.GetUserData() != nullptr) {
		throw XMLException(*this, "Element has user data but it should be empty");
	}

	for(const tinyxml2::XMLAttribute* attribute = element.FirstAttribute(); attribute != nullptr; attribute = attribute->Next()) {
		if(std::string(attribute->Name()) == "hostname") {
			hostname = attribute->Value();
			if(hostname == "") {
				throw XMLException(*this, "Invalid value \"\" for attribute 'hostname'");
			}
		}
		else if(std::string(attribute->Name()) == "ref-id") {
			refId = attribute->Value();
			if(refId == "") {
				throw XMLException(*this, "Invalid value \"\" for attribute 'ref-id'");
			}
		}
		else if(std::string(attribute->Name()) == "inherit") {
			std::string inheritStr = esl::utility::String::toLower(attribute->Value());
			if(inheritStr == "true") {
				inherit = true;
			}
			else if(inheritStr == "false") {
				inherit = false;
			}
			else {
				throw XMLException(*this, "Invalid value \"" + std::string(attribute->Value()) + "\" for attribute 'inherit'");
			}
		}
		else {
			throw XMLException(*this, "Unknown attribute '" + std::string(attribute->Name()) + "'");
		}
	}

	if(hostname == "") {
		throw XMLException(*this, "Attribute 'hostname' is missing");
	}
	if(refId == "") {
		throw XMLException(*this, "Attribute 'ref-id' is missing");
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

void Listener::save(std::ostream& oStream, std::size_t spaces) const {
	// 	<http-listener hostname="localhost" ref-id="http-1">
	oStream << makeSpaces(spaces) << "<http-listener hostname=\"" << hostname << "\" ref-id=\"" << refId << "\"";
	if(inherit) {
		oStream << " inherit=\"true\">\n";
	}
	else {
		oStream << " inherit=\"false\">\n";
	}

	for(const auto& entry : entries) {
		entry.save(oStream, spaces+2);
	}

	for(const auto& entry : responseHeaders) {
		entry.saveResponseHeader(oStream, spaces+2);
	}

	exceptions.save(oStream, spaces+2);

	oStream << makeSpaces(spaces) << "</http-listener>\n";
}

void Listener::install(engine::Engine& engine) const {
	engine::http::server::Context& context = engine.addHttpListener(refId, inherit, hostname);

	/* *****************
	 * install entries *
	 * *****************/
	for(const auto& entry : entries) {
		entry.install(context);
	}

	/* **********************
	 * Set response headers *
	 * **********************/
	for(const auto& responseHeader : responseHeaders) {
		context.addHeader(responseHeader.key, responseHeader.value);
	}

	exceptions.install(context);
}

void Listener::parseInnerElement(const tinyxml2::XMLElement& element) {
	if(element.Name() == nullptr) {
		throw XMLException(*this, "Element name is");
	}

	std::string innerElementName(element.Name());

	if(innerElementName == "response-header") {
		responseHeaders.push_back(Setting(getFileName(), element, false));
	}
	else if(innerElementName == "exceptions") {
		exceptions = Exceptions(getFileName(), element);
	}
	else {
		entries.push_back(Entry(getFileName(), element));
	}
}

} /* namespace http */
} /* namespace config */
} /* namespace jerry */
