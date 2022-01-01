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

#include <jerry/config/basic/Client.h>
#include <jerry/config/XMLException.h>
#include <jerry/Logger.h>

#include <esl/com/basic/client/Interface.h>

#include <stdexcept>

namespace jerry {
namespace config {
namespace basic {

namespace {
Logger logger("jerry::config::basic::Client");
} /* anonymous namespace */

Client::Client(const std::string& fileName, const tinyxml2::XMLElement& element)
: Config(fileName, element)
{
	if(element.GetUserData() != nullptr) {
		throw XMLException(*this, "Element has user data but it should be empty");
	}

	for(const tinyxml2::XMLAttribute* attribute = element.FirstAttribute(); attribute != nullptr; attribute = attribute->Next()) {
		if(std::string(attribute->Name()) == "id") {
			id = attribute->Value();
			if(id == "") {
				throw XMLException(*this, "Invalid value \"\" for attribute 'id'");
			}
		}
		else if(std::string(attribute->Name()) == "implementation") {
			implementation = attribute->Value();
			if(implementation == "") {
				throw XMLException(*this, "Invalid value \"\" for attribute 'implementation'");
			}
		}
		else {
			throw XMLException(*this, "Unknown attribute '" + std::string(attribute->Name()) + "'");
		}
	}

	if(id == "") {
		throw XMLException(*this, "Attribute 'id' is missing");
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
	oStream << makeSpaces(spaces) << "<basic-client id=\"" << id << "\"";
	if(implementation != "") {
		oStream << makeSpaces(spaces) << " implementation=\"" << implementation << "\"";
	}
	oStream << makeSpaces(spaces) << ">\n";

	for(const auto& entry : settings) {
		entry.saveParameter(oStream, spaces+2);
	}

	oStream << makeSpaces(spaces) << "<basic-client/>\n";
}

void Client::install(engine::ObjectContext& engineObjectContext) const {
	std::vector<std::pair<std::string, std::string>> eslSettings;
	for(const auto& setting : settings) {
		eslSettings.push_back(std::make_pair(setting.key, evaluate(setting.value, setting.language)));
	}

	logger.trace << "Adding basic-client (implementation=\"" << implementation << "\") with id=\"" << id << "\"\n";
	std::unique_ptr<esl::com::basic::client::Interface::ConnectionFactory> connectionFactory;
	try {
		connectionFactory = esl::getModule().getInterface<esl::com::basic::client::Interface>(implementation).createConnectionFactory(eslSettings);
	}
	catch(...) {
		throw XMLException(*this, "Could not create an connection-factory with id '" + id + "' for implementation '" + implementation + "'");
	}

	if(!connectionFactory) {
		throw std::runtime_error("Cannot create an basic connection-factory with id '" + id + "' for implementation '" + implementation + "' because interface method createConnectionFactory() returns nullptr.");
	}

	engineObjectContext.addObject(id, std::unique_ptr<esl::object::Interface::Object>(connectionFactory.release()));
}

void Client::parseInnerElement(const tinyxml2::XMLElement& element) {
	if(element.Name() == nullptr) {
		throw XMLException(*this, "Element name is empty");
	}

	std::string innerElementName(element.Name());

	if(innerElementName == "parameter") {
		settings.push_back(Setting(getFileName(), element, true));
	}
	else {
		throw XMLException(*this, "Unknown element name '" + std::string(element.Name()) + "'");
	}
}

} /* namespace basic */
} /* namespace config */
} /* namespace jerry */