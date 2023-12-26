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

#include <openjerry/config/main/Context.h>
#include <openjerry/config/FilePosition.h>
#include <openjerry/config/main/EntryImpl.h>
#include <openjerry/engine/main/Context.h>
#include <openjerry/utility/MIME.h>
#include <openjerry/Logger.h>

#include <esl/monitoring/Logger.h>
#include <esl/plugin/Registry.h>

#include <iostream>
#include <stdexcept>

namespace openjerry {
namespace config {
namespace main {

namespace {
Logger logger("openjerry::config::main::Context");
} /* anonymous namespace */

Context::Context(const std::string& configuration)
: Config("{mem}")
{
	tinyxml2::XMLError xmlError = xmlDocument.Parse(configuration.c_str(), configuration.size());
	if(xmlError != tinyxml2::XML_SUCCESS) {
		throw FilePosition::add(*this, xmlError);
	}

	const tinyxml2::XMLElement* element = xmlDocument.RootElement();
	if(element == nullptr) {
		throw FilePosition::add(*this, "No root element");
	}

	setXMLFile(getFileName(), *element);
	loadXML(*element);
}

Context::Context(const boost::filesystem::path& filename)
: Config(filename.generic_string())
{
	filesLoaded.insert(filename.generic_string());

	tinyxml2::XMLError xmlError = xmlDocument.LoadFile(filename.generic_string().c_str());
	if(xmlError != tinyxml2::XML_SUCCESS) {
		throw FilePosition::add(*this, xmlError);
	}

	const tinyxml2::XMLElement* element = xmlDocument.RootElement();
	if(element == nullptr) {
		throw FilePosition::add(*this, "No root element");
	}

	setXMLFile(filename.generic_string(), *element);
	loadXML(*element);
}

void Context::save(std::ostream& oStream) const {
	oStream << "\n<openjerry>\n";

	for(const auto& entry : libraries) {
		if(entry.second.empty()) {
			oStream << "  <library file=\"" << entry.first << "\"/>\n";
		}
		else {
			oStream << "  <library file=\"" << entry.first << "\" arguments=\"" << entry.second << "\"/>\n";
		}
	}

	for(const auto& entry : entries) {
		entry->save(oStream, 2);
	}

	oStream << "</openjerry>\n";
}


void Context::loadLibraries() {
	/* ************************
	 * load and add libraries *
	 * ********************** */
	for(auto& library : libraries) {
		esl::plugin::Registry::get().loadPlugin(library.first, library.second.c_str());
	}
}

void Context::install(engine::main::Context& context) {
	/*
	for(const auto& configCertificate : certificates) {
		context.addCertificate(configCertificate.domain, configCertificate.keyFile, configCertificate.certFile);
	}
	*/

	for(const auto& entry : entries) {
		entry->install(context);
	}
}

void Context::loadXML(const tinyxml2::XMLElement& element) {
	if(element.Name() == nullptr) {
		throw FilePosition::add(*this, "Name of XML root element is empty");
	}

	const std::string elementName(element.Name());

	if(elementName == "openjerry-batch") {
		std::cerr << "Tag <openjerry-batch> is deprecated. Use tag <openjerry> instead.";
	}
	else if(elementName == "openjerry-server") {
		std::cerr << "Tag <openjerry-server> is deprecated. Use tag <openjerry> instead.";
	}
	else if(elementName != "openjerry") {
		throw FilePosition::add(*this, "Name of XML root element is \"" + std::string(element.Name()) + "\" but should be \"openjerry\"");
	}

	if(element.GetUserData() != nullptr) {
		throw FilePosition::add(*this, "Node has user data but it should be empty");
	}

	for(const tinyxml2::XMLAttribute* attribute = element.FirstAttribute(); attribute != nullptr; attribute = attribute->Next()) {
		throw FilePosition::add(*this, "Unknown attribute '" + std::string(attribute->Name()) + "'");
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

void Context::parseInnerElement(const tinyxml2::XMLElement& element) {
	if(element.Name() == nullptr) {
		throw FilePosition::add(*this, "Element name is empty");
	}

	const std::string elementName(element.Name());

	if(elementName == "include") {
		parseInclude(element);
	}
	else if(elementName == "mime-types") {
		std::string file;

		if(element.GetUserData() != nullptr) {
			throw FilePosition::add(*this, "Element has user data but it should be empty");
		}

		for(const tinyxml2::XMLAttribute* attribute = element.FirstAttribute(); attribute != nullptr; attribute = attribute->Next()) {
			if(std::string(attribute->Name()) == "file") {
				file = attribute->Value();
				if(file == "") {
					throw FilePosition::add(*this, "Value \"\" of attribute 'file' is invalid.");
				}
			}
			else {
				throw FilePosition::add(*this, "Unknown attribute '" + std::string(attribute->Name()) + "'");
			}
		}

		if(file == "") {
			throw FilePosition::add(*this, "Missing attribute 'file'");
		}

		utility::MIME::loadDefinition(file);
	}
	else if(elementName == "library") {
		parseLibrary(element);
	}
	else if(elementName == "certificate") {
		Certificate(getFileName(), element);
	}
	else {
		entries.emplace_back(new EntryImpl(getFileName(), element));
	}
}

void Context::parseInclude(const tinyxml2::XMLElement& element) {
	std::string fileName;

	if(element.GetUserData() != nullptr) {
		throw FilePosition::add(*this, "Element has user data but it should be empty");
	}

	for(const tinyxml2::XMLAttribute* attribute = element.FirstAttribute(); attribute != nullptr; attribute = attribute->Next()) {
		if(std::string(attribute->Name()) == "file") {
			fileName = attribute->Value();
			if(fileName == "") {
				throw FilePosition::add(*this, "Value \"\" of attribute 'file' is invalid.");
			}
		}
		else {
			throw FilePosition::add(*this, "Unknown attribute '" + std::string(attribute->Name()) + "'");
		}
	}

	if(fileName == "") {
		throw FilePosition::add(*this, "Missing attribute 'file'");
	}

	if(filesLoaded.count(fileName) == 0) {
		auto oldXmlFile = setXMLFile(fileName, -1);
		filesLoaded.insert(fileName);

		tinyxml2::XMLDocument doc;
		tinyxml2::XMLError xmlError = doc.LoadFile(fileName.c_str());
		if(xmlError != tinyxml2::XML_SUCCESS) {
			throw FilePosition::add(*this, xmlError);
		}

		const tinyxml2::XMLElement* element = doc.RootElement();
		if(element == nullptr) {
			throw FilePosition::add(*this, "No root element");
		}

		setXMLFile(fileName, *element);
		loadXML(*element);
		setXMLFile(oldXmlFile);
	}
}

void Context::parseLibrary(const tinyxml2::XMLElement& element) {
	std::string fileName;
	std::string arguments;
	bool hasArguments = false;

	if(element.GetUserData() != nullptr) {
		throw FilePosition::add(*this, "Element has user data but it should be empty");
	}

	for(const tinyxml2::XMLAttribute* attribute = element.FirstAttribute(); attribute != nullptr; attribute = attribute->Next()) {
		if(std::string(attribute->Name()) == "file") {
			if(!fileName.empty()) {
				throw FilePosition::add(*this, "Multiple definition of attribute \"file\".");
			}
			fileName = attribute->Value();
			if(fileName.empty()) {
				throw FilePosition::add(*this, "Value \"\" of attribute 'file' is invalid.");
			}
		}
		else if(std::string(attribute->Name()) == "arguments") {
			if(hasArguments) {
				throw FilePosition::add(*this, "Multiple definition of attribute \"arguments\".");
			}
			arguments = attribute->Value();
			hasArguments = true;
		}
		else {
			throw FilePosition::add(*this, "Unknown attribute '" + std::string(attribute->Name()) + "'");
		}
	}

	if(fileName.empty()) {
		throw FilePosition::add(*this, "Missing attribute 'file'");
	}

	libraries.push_back(std::make_pair(fileName, arguments));
}

} /* namespace main */
} /* namespace config */
} /* namespace openjerry */
