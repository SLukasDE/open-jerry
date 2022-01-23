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

#include <jerry/config/http/HttpListener.h>
#include <jerry/config/http/EntryImpl.h>
#include <jerry/config/XMLException.h>
#include <jerry/engine/http/Context.h>

#include <esl/utility/String.h>

namespace jerry {
namespace config {
namespace http {

HttpListener::HttpListener(const std::string& fileName, const tinyxml2::XMLElement& element)
: Config(fileName, element)
{
	if(element.GetUserData() != nullptr) {
		throw jerry::config::XMLException(*this, "Element has user data but it should be empty");
	}

	bool hasInherit = false;

	for(const tinyxml2::XMLAttribute* attribute = element.FirstAttribute(); attribute != nullptr; attribute = attribute->Next()) {
		if(std::string(attribute->Name()) == "inherit") {
			std::string inheritStr = esl::utility::String::toLower(attribute->Value());
			if(hasInherit) {
				throw jerry::config::XMLException(*this, "Multiple definition of attribute 'inherit'");
			}
			hasInherit = true;
			if(inheritStr == "true") {
				inherit = true;
			}
			else if(inheritStr == "false") {
				inherit = false;
			}
			else {
				throw jerry::config::XMLException(*this, "Invalid value \"" + std::string(attribute->Value()) + "\" for attribute 'inherit'");
			}
		}
		else {
			throw jerry::config::XMLException(*this, "Unknown attribute '" + std::string(attribute->Name()) + "'");
		}
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

void HttpListener::save(std::ostream& oStream, std::size_t spaces) const {
	oStream << makeSpaces(spaces) << "<http-listener";

	if(inherit) {
		oStream << " inherit=\"true\"";
	}
	else {
		oStream << " inherit=\"false\"";
	}
	oStream << ">\n";

	for(const auto& entry : entries) {
		entry->save(oStream, spaces+2);
	}

	for(const auto& entry : responseHeaders) {
		entry.saveResponseHeader(oStream, spaces+2);
	}

	exceptions.save(oStream, spaces+2);

	oStream << makeSpaces(spaces) << "</http-listener>\n";
}

void HttpListener::install(engine::Application& engineApplication) const {
	engine::http::Context& newEngineContext = engineApplication.addHttpListener();

	if(inherit) {
		newEngineContext.ObjectContext::setParent(&engineApplication);
	}

	/* *****************
	 * install entries *
	 * *****************/
	for(const auto& entry : entries) {
		entry->install(newEngineContext);
	}

	/* **********************
	 * Set response headers *
	 * **********************/
	for(const auto& responseHeader : responseHeaders) {
		newEngineContext.addHeader(responseHeader.key, responseHeader.value);
	}

	exceptions.install(newEngineContext);
}

void HttpListener::parseInnerElement(const tinyxml2::XMLElement& element) {
	std::string innerElementName(element.Name());

	if(innerElementName == "response-header") {
		responseHeaders.push_back(jerry::config::Setting(getFileName(), element, false));
	}
	else if(innerElementName == "exceptions") {
		exceptions = jerry::config::http::Exceptions(getFileName(), element);
	}
	else {
		entries.emplace_back(new jerry::config::http::EntryImpl(getFileName(), element));
	}
}

} /* namespace http */
} /* namespace config */
} /* namespace jerry */