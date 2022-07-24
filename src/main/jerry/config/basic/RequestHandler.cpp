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

#include <jerry/config/basic/RequestHandler.h>
#include <jerry/config/FilePosition.h>

#include <esl/plugin/exception/PluginNotFound.h>
#include <esl/plugin/Registry.h>

#include <stdexcept>
#include <utility>

namespace jerry {
namespace config {
namespace basic {

RequestHandler::RequestHandler(const std::string& fileName, const tinyxml2::XMLElement& element)
: Config(fileName, element)
{
	if(element.GetUserData() != nullptr) {
		throw FilePosition::add(*this, "Element has user data but it should be empty");
	}

	for(const tinyxml2::XMLAttribute* attribute = element.FirstAttribute(); attribute != nullptr; attribute = attribute->Next()) {
		if(std::string(attribute->Name()) == "implementation") {
			if(!implementation.empty()) {
				throw std::runtime_error("Multiple definition of attribute 'implementation'.");
			}
			implementation = attribute->Value();
			if(implementation.empty()) {
				throw FilePosition::add(*this, "Invalid value \"\" for attribute 'implementation'");
			}
		}
		else {
			throw FilePosition::add(*this, "Unknown attribute '" + std::string(attribute->Name()) + "'");
		}
	}

	if(implementation.empty()) {
		throw FilePosition::add(*this, "Attribute 'implementation' is missing");
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

void RequestHandler::save(std::ostream& oStream, std::size_t spaces) const {
	oStream << makeSpaces(spaces) << "<requesthandler implementation=\"" << implementation << "\">\n";

	for(const auto& entry : settings) {
		entry.saveParameter(oStream, spaces+2);
	}

	oStream << makeSpaces(spaces) << "</requesthandler>\n";
}

void RequestHandler::install(engine::basic::Context& context) const {
#if 1
	context.addRequestHandler(create());
#else
	std::vector<std::pair<std::string, std::string>> eslSettings;
	for(const auto& setting : settings) {
		eslSettings.push_back(std::make_pair(setting.key, evaluate(setting.value, setting.language)));
	}

	try {
		context.addRequestHandler(implementation, eslSettings);
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
		throw FilePosition::add(*this, "Could not create basic-request-handler for implementation '" + implementation + "' because an unknown exception occurred.");
	}
#endif
}

void RequestHandler::parseInnerElement(const tinyxml2::XMLElement& element) {
	if(element.Name() == nullptr) {
		throw FilePosition::add(*this, "Element name is empty");
	}

	std::string elementName(element.Name());

	if(elementName == "parameter") {
		settings.push_back(Setting(getFileName(), element, true));
	}
	else {
		throw FilePosition::add(*this, "Unknown element name \"" + elementName + "\"");
	}
}

std::unique_ptr<esl::com::basic::server::RequestHandler> RequestHandler::create() const {
	std::vector<std::pair<std::string, std::string>> eslSettings;
	for(const auto& setting : settings) {
		eslSettings.push_back(std::make_pair(setting.key, evaluate(setting.value, setting.language)));
	}

	std::unique_ptr<esl::com::basic::server::RequestHandler> requestHandler;
	try {
		requestHandler = esl::plugin::Registry::get().create<esl::com::basic::server::RequestHandler>(implementation, eslSettings);
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
		throw FilePosition::add(*this, "Could not create basic-request-handler for implementation '" + implementation + "' because an unknown exception occurred.");
	}

	if(!requestHandler) {
		throw FilePosition::add(*this, "Could not create basic-request-handler for implementation '" + implementation + "' because interface method createRequestHandler() returns nullptr.");
	}

	return requestHandler;
}

} /* namespace basic */
} /* namespace config */
} /* namespace jerry */
