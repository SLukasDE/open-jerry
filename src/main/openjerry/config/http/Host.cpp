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

#include <openjerry/config/http/Host.h>
#include <openjerry/config/http/EntryImpl.h>
#include <openjerry/config/http/Context.h>
#include <openjerry/config/http/RequestHandler.h>
#include <openjerry/config/http/Entry.h>
#include <openjerry/config/Object.h>
#include <openjerry/config/FilePosition.h>
#include <openjerry/engine/http/Host.h>

#include <esl/utility/String.h>

namespace openjerry {
namespace config {
namespace http {

Host::Host(const std::string& fileName, const tinyxml2::XMLElement& element)
: Config(fileName, element)
{
	if(element.GetUserData() != nullptr) {
		throw FilePosition::add(*this, "Element has user data but it should be empty");
	}

	for(const tinyxml2::XMLAttribute* attribute = element.FirstAttribute(); attribute != nullptr; attribute = attribute->Next()) {
		if(std::string(attribute->Name()) == "server-name") {
			serverName = attribute->Value();
			if(serverName == "") {
				throw FilePosition::add(*this, "Invalid value \"\" for attribute 'server-name'");
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
				throw FilePosition::add(*this, "Invalid value \"" + std::string(attribute->Value()) + "\" for attribute 'inherit'");
			}
		}
		else {
			throw FilePosition::add(*this, "Unknown attribute '" + std::string(attribute->Name()) + "'");
		}
	}

	if(serverName == "") {
		throw FilePosition::add(*this, "Missing attribute 'server-name'");
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

void Host::save(std::ostream& oStream, std::size_t spaces) const {
	oStream << makeSpaces(spaces) << "<host server-name=\"" << serverName << "\">\n";

	for(const auto& entry : entries) {
		entry->save(oStream, spaces+2);
	}

	for(const auto& responseHeader : responseHeaders) {
		responseHeader.saveResponseHeader(oStream, spaces+2);
	}

	exceptions.save(oStream, spaces+2);

	oStream << makeSpaces(spaces) << "<endpoint/>\n";
}

void Host::install(engine::http::Context& engineHttpContext) const {
	std::unique_ptr<engine::http::Host> httpHost(new engine::http::Host(engineHttpContext.getProcessRegistry(), serverName));
	engine::http::Host& httpHostRef = *httpHost;

	if(inherit) {
		httpHostRef.setParent(&engineHttpContext);
	}

	engineHttpContext.addHost(std::move(httpHost));


	/* *****************
	 * install entries *
	 * *****************/
	for(const auto& entry : entries) {
		entry->install(httpHostRef);
	}

	/* **********************
	 * Set response headers *
	 * **********************/
	for(const auto& responseHeader : responseHeaders) {
		httpHostRef.addHeader(responseHeader.key, responseHeader.value);
	}

	exceptions.install(httpHostRef);
}

void Host::parseInnerElement(const tinyxml2::XMLElement& element) {
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
