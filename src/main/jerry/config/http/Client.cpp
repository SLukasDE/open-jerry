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

#include <jerry/config/http/Client.h>
#include <jerry/config/XMLException.h>
#include <jerry/Logger.h>

#include <esl/com/http/client/Interface.h>

#include <stdexcept>

namespace jerry {
namespace config {
namespace http {

namespace {
Logger logger("jerry::config::http::Client");
} /* anonymous namespace */

Client::Client(const std::string& fileName, const tinyxml2::XMLElement& element)
: Config(fileName, element)
{
	if(element.GetUserData() != nullptr) {
		throw XMLException(*this, "Element has user data but it should be empty");
	}

	for(const tinyxml2::XMLAttribute* attribute = element.FirstAttribute(); attribute != nullptr; attribute = attribute->Next()) {
		// 	<http-client id="http-1" implementation="curl4esl" url="http://www.host.bla/subpage">
		if(std::string(attribute->Name()) == "id") {
			id = attribute->Value();
			if(id == "") {
				throw XMLException(*this, "Value \"\" of attribute 'id' is invalid.");
			}
		}
		else if(std::string(attribute->Name()) == "implementation") {
			implementation = attribute->Value();
			if(implementation == "") {
				throw XMLException(*this, "Value \"\" of attribute 'implementation' is invalid.");
			}
		}
		else {
			throw XMLException(*this, "Unknown attribute '" + std::string(attribute->Name()) + "'");
		}
	}

	if(id == "") {
		throw XMLException(*this, "Missing attribute 'id'");
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

void Client::save(std::ostream& oStream, std::size_t spaces) const {
	oStream << makeSpaces(spaces) << "<http-client id=\"" << id << "\"";
	if(implementation != "") {
		oStream <<  " implementation=\"" << implementation << "\"";
	}
	oStream << ">\n";

	for(const auto& entry : settings) {
		entry.saveParameter(oStream, spaces+2);
	}

	oStream << makeSpaces(spaces) << "</http-client>\n";
}

void Client::install(engine::ObjectContext& engineObjectContext) const {
	engineObjectContext.addObject(id, install());
}

std::unique_ptr<esl::object::Interface::Object> Client::install() const {
	std::vector<std::pair<std::string, std::string>> eslSettings;
	for(const auto& setting : settings) {
		eslSettings.push_back(std::make_pair(setting.key, evaluate(setting.value, setting.language)));
	}

	logger.trace << "Adding http-client (implementation=\"" << implementation << "\") with id=\"" << id << "\"\n";
	std::unique_ptr<esl::com::http::client::Interface::ConnectionFactory> connectionFactory;
	try {
		connectionFactory = esl::getModule().getInterface<esl::com::http::client::Interface>(implementation).createConnectionFactory(eslSettings);
	}
	catch(const std::exception& e) {
		throw XMLException(*this, e.what());
	}
	catch(...) {
		throw XMLException(*this, "Could not create a http connection-factory with id '" + id + "' for implementation '" + implementation + "' because an unknown exception occurred.");
	}

	if(!connectionFactory) {
		throw std::runtime_error("Cannot create a http connection-factory with id '" + id + "' for implementation '" + implementation + "' because interface method createConnectionFactory() returns nullptr.");
	}

	return std::unique_ptr<esl::object::Interface::Object>(connectionFactory.release());
}

void Client::parseInnerElement(const tinyxml2::XMLElement& element) {
	if(element.Name() == nullptr) {
		throw XMLException(*this, "Element name is empty");
	}

	std::string elementName(element.Name());

	if(elementName == "parameter") {
		settings.push_back(Setting(getFileName(), element, true));
	}
	else {
		throw XMLException(*this, "Unknown element name \"" + elementName + "\"");
	}
}

} /* namespace http */
} /* namespace config */
} /* namespace jerry */
