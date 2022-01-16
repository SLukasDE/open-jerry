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

#include <jerry/config/main/Engine.h>
#include <jerry/config/XMLException.h>
#include <jerry/utility/MIME.h>
#include <jerry/Logger.h>

#include <esl/Module.h>
#include <esl/logging/appender/Appender.h>
#include <esl/logging/layout/Layout.h>
#include <esl/logging/Logger.h>

namespace jerry {
namespace config {
namespace main {

namespace {
Logger logger("jerry::config::main::Engine");
} /* anonymous namespace */

Engine::Engine(const std::string& fileName)
: Config(fileName)
{
	filesLoaded.insert(fileName);

	tinyxml2::XMLError xmlError = xmlDocument.LoadFile(fileName.c_str());
	if(xmlError != tinyxml2::XML_SUCCESS) {
		throw XMLException(*this, xmlError);
	}

	const tinyxml2::XMLElement* element = xmlDocument.RootElement();
	if(element == nullptr) {
		throw XMLException(*this, "No root element");
	}

	setXMLFile(fileName, *element);
	loadXML(*element);
}

void Engine::save(std::ostream& oStream) const {
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

	loggerConfig.save(oStream, 2);

	for(const auto& entry : entries) {
		entry->save(oStream, 2);
	}

	oStream << "</jerry>\n";
}


void Engine::loadLibraries() {
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

std::unique_ptr<esl::logging::layout::Interface::Layout> Engine::install(engine::Engine& engine, esl::logging::appender::Interface::Appender& appenderCoutStream, esl::logging::appender::Interface::Appender& appenderMemBuffer) {
#if 0
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
#endif

	/* ************* *
	 * create layout *
	 * ************* */
	esl::object::Interface::Settings eslSettings;

	for(auto const setting : loggerConfig.layoutSettings) {
		eslSettings.push_back(std::make_pair(setting.key, setting.value));
	}

	std::unique_ptr<esl::logging::layout::Interface::Layout> layout;
	try {
		layout.reset(new esl::logging::layout::Layout(eslSettings, loggerConfig.layout));
	}
	catch(const std::exception& e) {
		throw XMLException(*this, e.what());
	}
	catch(...) {
		throw XMLException(*this, "Could not create logging-layout for implementation '" + loggerConfig.layout + "' because an unknown exception occurred.");
	}


	/* *********************** *
	 * set layout to appenders *
	 * *********************** */

    appenderCoutStream.setRecordLevel();
    appenderCoutStream.setLayout(layout.get());
    esl::logging::addAppender(appenderCoutStream);

    /* MemBuffer appender just writes output to a buffer of a fixed number of lines.
     * If number of columns is specified as well the whole memory is allocated at initialization time.
     */
    appenderMemBuffer.setRecordLevel(esl::logging::appender::Interface::Appender::RecordLevel::ALL);
    appenderMemBuffer.setLayout(layout.get());
    esl::logging::addAppender(appenderMemBuffer);


    /* ************** *
     * set log levels *
     * ************** */
	for(auto const setting : loggerConfig.levelSettings) {
		if(setting.level == "SILENT") {
			esl::logging::setLevel(esl::logging::Level::SILENT, setting.className);
		}
		else if(setting.level == "ERROR") {
			esl::logging::setLevel(esl::logging::Level::ERROR, setting.className);
		}
		else if(setting.level == "WARN") {
			esl::logging::setLevel(esl::logging::Level::WARN, setting.className);
		}
		else if(setting.level == "INFO") {
			esl::logging::setLevel(esl::logging::Level::INFO, setting.className);
		}
		else if(setting.level == "DEBUG") {
			esl::logging::setLevel(esl::logging::Level::DEBUG, setting.className);
		}
		else if(setting.level == "TRACE") {
			esl::logging::setLevel(esl::logging::Level::TRACE, setting.className);
		}
		else {
			logger.warn << "Unknown logging level \"" << setting.level << "\" for class \"" << setting.className << "\"\n";
		}
	}


	for(const auto& configCertificate : certificates) {
		engine.addCertificate(configCertificate.domain, configCertificate.keyFile, configCertificate.certFile);
	}

	for(const auto& entry : entries) {
		entry->install(engine);
	}

	return layout;
}

void Engine::loadXML(const tinyxml2::XMLElement& element) {
	if(element.Name() == nullptr) {
		throw XMLException(*this, "Name of XML root element is empty");
	}
	if(std::string(element.Name()) != "jerry") {
		throw XMLException(*this, "Name of XML root element is \"" + std::string(element.Name()) + "\" but should be \"jerry\"");
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

void Engine::parseInnerElement(const tinyxml2::XMLElement& element) {
	if(element.Name() == nullptr) {
		throw XMLException(*this, "Element name is empty");
	}

	std::string elementName(element.Name());

	if(elementName == "include") {
		parseInclude(element);
	}
	else if(elementName == "mime-types") {
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
	else if(elementName == "certificate") {
		certificates.push_back(Certificate(getFileName(), element));
	}
	else if(elementName == "logger") {
		loggerConfig = LoggerConfig(getFileName(), element);
	}
	else {
		entries.emplace_back(new Entry(getFileName(), element));
	}
}

void Engine::parseInclude(const tinyxml2::XMLElement& element) {
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

void Engine::parseLibrary(const tinyxml2::XMLElement& element) {
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
