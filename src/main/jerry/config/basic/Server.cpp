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

#include <jerry/config/basic/Server.h>
#include <jerry/config/basic/EntryImpl.h>
#include <jerry/config/FilePosition.h>
#include <jerry/engine/basic/Server.h>
#include <jerry/Logger.h>

#include <esl/plugin/exception/PluginNotFound.h>
#include <esl/utility/String.h>

#include <stdexcept>

namespace jerry {
namespace config {
namespace basic {

namespace {
Logger logger("jerry::config::basic::Server");
} /* anonymous namespace */

Server::Server(const std::string& fileName, const tinyxml2::XMLElement& element)
: Config(fileName, element)
{
	if(element.GetUserData() != nullptr) {
		throw FilePosition::add(*this, "Element has user data but it should be empty");
	}

	bool hasInherit = false;

	for(const tinyxml2::XMLAttribute* attribute = element.FirstAttribute(); attribute != nullptr; attribute = attribute->Next()) {
		if(std::string(attribute->Name()) == "implementation") {
			implementation = attribute->Value();
			if(implementation == "") {
				throw FilePosition::add(*this, "Invalid value \"\" for attribute 'implementation'");
			}
		}
		else if(std::string(attribute->Name()) == "inherit") {
			if(hasInherit) {
				throw FilePosition::add(*this, "Multiple definition of attribute 'inherit'");
			}
			std::string inheritStr = esl::utility::String::toLower(attribute->Value());
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

void Server::save(std::ostream& oStream, std::size_t spaces) const {
	oStream << makeSpaces(spaces) << "<basic-server";
	if(implementation != "") {
		oStream << makeSpaces(spaces) << " implementation=\"" << implementation << "\"";
	}

	if(inherit) {
		oStream << " inherit=\"true\"";
	}
	else {
		oStream << " inherit=\"false\"";
	}

	oStream << ">\n";

	for(const auto& entry : settings) {
		entry.saveParameter(oStream, spaces+2);
	}

	for(const auto& entry : entries) {
		entry->save(oStream, spaces+2);
	}

	oStream << makeSpaces(spaces) << "</basic-server>\n";
}

void Server::install(engine::main::Context& engineMainContext) const {
	std::vector<std::pair<std::string, std::string>> eslSettings;

	for(const auto& setting : settings) {
		eslSettings.push_back(std::make_pair(setting.key, evaluate(setting.value, setting.language)));
	}

	try {
		logger.trace << "Adding basic server (implementation=\"" << implementation << "\")\n";

		std::unique_ptr<engine::basic::Server> server(new engine::basic::Server(engineMainContext, eslSettings, implementation));
		engine::basic::Server& serverRef = *server;

		if(inherit) {
			serverRef.getContext().ObjectContext::setParent(&engineMainContext);
		}

		engineMainContext.addBasicServer(std::move(server));

		/* *****************
		 * install entries *
		 * *****************/
		for(const auto& entry : entries) {
			entry->install(serverRef.getContext());
		}
		//listener->install(engineMainContext, serverRef);
	}
	catch(const esl::plugin::exception::PluginNotFound& e) {
		throw FilePosition::add(*this, e);
	}
	catch(const std::runtime_error& e) {
		throw FilePosition::add(*this, e);
	}
	catch(const std::exception& e) {
		throw FilePosition::add(*this, e);
	}
	catch(...) {
		throw FilePosition::add(*this, "Could not create basic-socket for implementation '" + implementation + "' because an unknown exception occurred.");
	}
}

void Server::parseInnerElement(const tinyxml2::XMLElement& element) {
	if(element.Name() == nullptr) {
		throw FilePosition::add(*this, "Element name is empty");
	}

	std::string innerElementName(element.Name());

	if(innerElementName == "parameter") {
		settings.push_back(Setting(getFileName(), element, true));
	}
	else {
		entries.emplace_back(new EntryImpl(getFileName(), element));
	}
}

} /* namespace basic */
} /* namespace config */
} /* namespace jerry */
