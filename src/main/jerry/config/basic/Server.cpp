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

#include <jerry/config/basic/Server.h>
#include <jerry/config/Engine.h>
#include <jerry/config/XMLException.h>

#include <esl/Stacktrace.h>
#include <esl/utility/String.h>

#include <stdexcept>

namespace jerry {
namespace config {
namespace basic {

Server::Server(const std::string& fileName, const tinyxml2::XMLElement& element)
: Config(fileName, element)
{
	if(element.GetUserData() != nullptr) {
		throw XMLException(*this, "Element has user data but it should be empty");
	}

	for(const tinyxml2::XMLAttribute* attribute = element.FirstAttribute(); attribute != nullptr; attribute = attribute->Next()) {
		if(std::string(attribute->Name()) == "implementation") {
			implementation = attribute->Value();
			if(implementation == "") {
				throw XMLException(*this, "Invalid value \"\" for attribute 'implementation'");
			}
		}
		else {
			throw XMLException(*this, "Unknown attribute '" + std::string(attribute->Name()) + "'");
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

	if(!listener) {
		throw XMLException(*this, "Missing definition of element \"listener\"");
	}
}

void Server::save(std::ostream& oStream, std::size_t spaces) const {
	oStream << makeSpaces(spaces) << "<basic-server";
	if(implementation != "") {
		oStream << makeSpaces(spaces) << " implementation=\"" << implementation << "\"";
	}
	oStream << makeSpaces(spaces) << ">\n";

	for(const auto& entry : settings) {
		entry.saveParameter(oStream, spaces+2);
	}

	listener->save(oStream, spaces+2);

	oStream << makeSpaces(spaces) << "<basic-server/>\n";
}

void Server::install(engine::Engine& jEngine) const {
	std::vector<std::pair<std::string, std::string>> eslSettings;

	for(const auto& setting : settings) {
		eslSettings.push_back(std::make_pair(setting.key, evaluate(setting.value, setting.language)));
	}

	engine::basic::server::Context& newEngineHttpContext = jEngine.addBasicServer(eslSettings, implementation);

	listener->install(newEngineHttpContext);
}

void Server::parseInnerElement(const tinyxml2::XMLElement& element) {
	if(element.Name() == nullptr) {
		throw XMLException(*this, "Element name is empty");
	}

	std::string innerElementName(element.Name());

	if(innerElementName == "parameter") {
		settings.push_back(Setting(getFileName(), element, true));
	}
	else if(innerElementName == "listener") {
		if(listener) {
			throw XMLException(*this, "Multiple definition of element \"listener\"");
		}
		listener = std::unique_ptr<Context>(new Context(getFileName(), element, Context::listener));
	}
	else {
		throw XMLException(*this, "Unknown element name '" + std::string(element.Name()) + "'");
	}
}

} /* namespace basic */
} /* namespace config */
} /* namespace jerry */
