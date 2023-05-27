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

#include <openjerry/config/FilePosition.h>
#include <openjerry/config/http/Server.h>
#include <openjerry/config/http/Exceptions.h>
#include <openjerry/config/http/EntryImpl.h>
#include <openjerry/engine/http/Server.h>
#include <openjerry/Logger.h>

#include <esl/plugin/exception/PluginNotFound.h>
#include <esl/utility/String.h>

namespace openjerry {
namespace config {
namespace http {

namespace {
Logger logger("openjerry::config::http::Server");
} /* anonymous namespace */

Server::Server(const std::string& fileName, const tinyxml2::XMLElement& element)
: Config(fileName, element)
{
	if(element.GetUserData() != nullptr) {
		throw FilePosition::add(*this, "Element has user data but it should be empty");
	}

	bool hasInherit = false;

	for(const tinyxml2::XMLAttribute* attribute = element.FirstAttribute(); attribute != nullptr; attribute = attribute->Next()) {
		// 	<http-server implementation="mhd4esl" https="true">
		if(std::string(attribute->Name()) == "implementation") {
			implementation = attribute->Value();
			if(implementation == "") {
				throw FilePosition::add(*this, "Value \"\" of attribute 'implementation' is invalid.");
			}
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
				throw FilePosition::add(*this, "Invalid value \"" + std::string(attribute->Value()) + "\" for attribute 'https'");
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
			//throw FilePosition::add(*this, "Unknown attribute '" + std::string(attribute->Name()) + "'");
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
	oStream << makeSpaces(spaces) << "<http-server";
	if(implementation != "") {
		oStream <<  " implementation=\"" << implementation << "\"";
	}

	if(isHttps) {
		oStream << " https=\"true\"";
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

	for(const auto& responseHeader : responseHeaders) {
		responseHeader.saveResponseHeader(oStream, spaces+2);
	}

	exceptions.save(oStream, spaces+2);

	oStream << makeSpaces(spaces) << "</http-server>\n";
}

void Server::install(engine::main::Context& engineMainContext) const {
	std::vector<std::pair<std::string, std::string>> eslSettings;

	for(const auto& setting : settings) {
		eslSettings.push_back(std::make_pair(setting.key, evaluate(setting.value, setting.language)));
	}

	try {
		if(isHttps) {
			logger.trace << "Adding HTTPS server (implementation=\"" << implementation << "\")\n";
		}
		else {
			logger.trace << "Adding HTTP server (implementation=\"" << implementation << "\")\n";
		}

		std::unique_ptr<engine::http::Server> server(new engine::http::Server(engineMainContext, isHttps, eslSettings, implementation));
		engine::http::Server& serverRef = *server;

		if(inherit) {
			serverRef.getContext().ObjectContext::setParent(&engineMainContext);
		}

		engineMainContext.addHttpServer(std::move(server));

		/* *****************
		 * install entries *
		 * *****************/
		for(const auto& entry : entries) {
			entry->install(serverRef.getContext());
		}

		/* **********************
		 * Set response headers *
		 * **********************/
		for(const auto& responseHeader : responseHeaders) {
			serverRef.getContext().addHeader(responseHeader.key, responseHeader.value);
		}

		exceptions.install(serverRef.getContext());
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
		throw FilePosition::add(*this, "Could not create http-socket for implementation '" + implementation + "' because an unknown exception occurred.");
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
	else if(innerElementName == "response-header") {
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
