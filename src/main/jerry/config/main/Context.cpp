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

#include <jerry/config/main/Context.h>
#include <jerry/config/XMLException.h>
#include <jerry/config/main/EntryImpl.h>
#include <jerry/engine/main/Context.h>
#include <jerry/utility/MIME.h>
#include <jerry/Logger.h>

#include <esl/Module.h>
#include <esl/logging/appender/Appender.h>
#include <esl/logging/layout/Layout.h>
#include <esl/logging/Logger.h>

#include <iostream>
#include <stdexcept>

namespace jerry {
namespace config {
namespace main {

namespace {
Logger logger("jerry::config::main::Context");
} /* anonymous namespace */

Context::Context(const std::string& configuration, bool aIsJBoot)
: Config("{mem}"),
  isJBoot(aIsJBoot)
{
	tinyxml2::XMLError xmlError = xmlDocument.Parse(configuration.c_str(), configuration.size());
	if(xmlError != tinyxml2::XML_SUCCESS) {
		throw XMLException(*this, xmlError);
	}

	const tinyxml2::XMLElement* element = xmlDocument.RootElement();
	if(element == nullptr) {
		throw XMLException(*this, "No root element");
	}

	setXMLFile(getFileName(), *element);
	loadXML(*element);
}

Context::Context(const boost::filesystem::path& filename, bool aIsJBoot)
: Config(filename.generic_string()),
  isJBoot(aIsJBoot)
{
	filesLoaded.insert(filename.generic_string());

	tinyxml2::XMLError xmlError = xmlDocument.LoadFile(filename.generic_string().c_str());
	if(xmlError != tinyxml2::XML_SUCCESS) {
		throw XMLException(*this, xmlError);
	}

	const tinyxml2::XMLElement* element = xmlDocument.RootElement();
	if(element == nullptr) {
		throw XMLException(*this, "No root element");
	}

	setXMLFile(filename.generic_string(), *element);
	loadXML(*element);
}

void Context::save(std::ostream& oStream) const {
	oStream << "\n<jerry>\n";

	for(const auto& entry : libraries) {
		oStream << "  <library file=\"" << entry.first << "\"/>\n";
	}
	/*
	for(const auto& entry : filesLoaded) {
		oStream << "  <include file=\"" << entry << "\"/>\n";
	}
	*/
	for(const auto& entry : certificates) {
		entry.save(oStream, 2);
	}

	for(const auto& entry : entries) {
		entry->save(oStream, 2);
	}

	oStream << "</jerry>\n";
}


void Context::loadLibraries() {
	/* ************************
	 * load and add libraries *
	 * ********************** */
	for(auto& library : libraries) {
		/*
		if(library.second) {
			throw esl::addStacktrace(std::runtime_error(std::string("Library \"") + library.first + "\" loaded already."));
		}
		*/
		library.second = &esl::module::Library::load(library.first);
		library.second->install(esl::getModule());
	}
}

void Context::install(engine::main::Context& context) {
	for(const auto& eslLogger : eslLoggers) {
		eslLogger.install();
	}

	for(const auto& configCertificate : certificates) {
		context.addCertificate(configCertificate.domain, configCertificate.keyFile, configCertificate.certFile);
	}

	for(const auto& entry : entries) {
		entry->install(context);
	}
}

void Context::loadXML(const tinyxml2::XMLElement& element) {
	if(element.Name() == nullptr) {
		throw XMLException(*this, "Name of XML root element is empty");
	}

	const std::string elementName(element.Name());

	if(isJBoot) {
		if(elementName != "jboot") {
			throw XMLException(*this, "Name of XML root element is \"" + std::string(element.Name()) + "\" but should be \"jboot\"");
		}
	}
	else {
		if(elementName == "jerry-batch") {
			std::cerr << "Tag <jerry-batch> is deprecated. Use tag <jerry> instead.";
		}
		else if(elementName == "jerry-server") {
			std::cerr << "Tag <jerry-server> is deprecated. Use tag <jerry> instead.";
		}
		else if(elementName != "jerry") {
			throw XMLException(*this, "Name of XML root element is \"" + std::string(element.Name()) + "\" but should be \"jerry\"");
		}
	}

	if(element.GetUserData() != nullptr) {
		throw XMLException(*this, "Node has user data but it should be empty");
	}

	for(const tinyxml2::XMLAttribute* attribute = element.FirstAttribute(); attribute != nullptr; attribute = attribute->Next()) {
		throw XMLException(*this, "Unknown attribute '" + std::string(attribute->Name()) + "'");
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
		throw XMLException(*this, "Element name is empty");
	}

	const std::string elementName(element.Name());

	if(elementName == "include") {
		parseInclude(element);
	}
	else if(elementName == "mime-types" && isJBoot == false) {
		std::string file;

		if(element.GetUserData() != nullptr) {
			throw XMLException(*this, "Element has user data but it should be empty");
		}

		for(const tinyxml2::XMLAttribute* attribute = element.FirstAttribute(); attribute != nullptr; attribute = attribute->Next()) {
			if(std::string(attribute->Name()) == "file") {
				file = attribute->Value();
				if(file == "") {
					throw XMLException(*this, "Value \"\" of attribute 'file' is invalid.");
				}
			}
			else {
				throw XMLException(*this, "Unknown attribute '" + std::string(attribute->Name()) + "'");
			}
		}

		if(file == "") {
			throw XMLException(*this, "Missing attribute 'file'");
		}

		utility::MIME::loadDefinition(file);
	}
	else if(elementName == "library") {
		parseLibrary(element);
	}
	else if(elementName == "certificate" && isJBoot == false) {
		certificates.push_back(Certificate(getFileName(), element));
	}
	else if(elementName == "logger" && isJBoot == false) {
		std::cerr << "Tag <logger> is deprecated. Use tag <esl-logger> instead or use a separate esl-logger configuration file outside of the jerry configuration file. Skipping tag <logger> !\n";
	}
	else if(elementName == "esl-logger") {
		eslLoggers.push_back(logging::Logger(getFileName(), element));
	}
	else {
		entries.emplace_back(new EntryImpl(getFileName(), element, isJBoot));
	}
}

void Context::parseInclude(const tinyxml2::XMLElement& element) {
	std::string fileName;

	if(element.GetUserData() != nullptr) {
		throw XMLException(*this, "Element has user data but it should be empty");
	}

	for(const tinyxml2::XMLAttribute* attribute = element.FirstAttribute(); attribute != nullptr; attribute = attribute->Next()) {
		if(std::string(attribute->Name()) == "file") {
			fileName = attribute->Value();
			if(fileName == "") {
				throw XMLException(*this, "Value \"\" of attribute 'file' is invalid.");
			}
		}
		else {
			throw XMLException(*this, "Unknown attribute '" + std::string(attribute->Name()) + "'");
		}
	}

	if(fileName == "") {
		throw XMLException(*this, "Missing attribute 'file'");
	}

	if(filesLoaded.count(fileName) == 0) {
		auto oldXmlFile = setXMLFile(fileName, -1);
		filesLoaded.insert(fileName);

		tinyxml2::XMLDocument doc;
		tinyxml2::XMLError xmlError = doc.LoadFile(fileName.c_str());
		if(xmlError != tinyxml2::XML_SUCCESS) {
			throw XMLException(*this, xmlError);
		}

		const tinyxml2::XMLElement* element = doc.RootElement();
		if(element == nullptr) {
			throw XMLException(*this, "No root element");
		}

		setXMLFile(fileName, *element);
		loadXML(*element);
		setXMLFile(oldXmlFile);
	}
}

void Context::parseLibrary(const tinyxml2::XMLElement& element) {
	std::string fileName;

	if(element.GetUserData() != nullptr) {
		throw XMLException(*this, "Element has user data but it should be empty");
	}

	for(const tinyxml2::XMLAttribute* attribute = element.FirstAttribute(); attribute != nullptr; attribute = attribute->Next()) {
		if(std::string(attribute->Name()) == "file") {
			fileName = attribute->Value();
			if(fileName == "") {
				throw XMLException(*this, "Value \"\" of attribute 'file' is invalid.");
			}
		}
		else {
			throw XMLException(*this, "Unknown attribute '" + std::string(attribute->Name()) + "'");
		}
	}

	if(fileName == "") {
		throw XMLException(*this, "Missing attribute 'file'");
	}

	libraries.push_back(std::make_pair(fileName, nullptr));
}

} /* namespace main */
} /* namespace config */
} /* namespace jerry */
