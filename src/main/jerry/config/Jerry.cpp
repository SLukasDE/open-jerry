/*
 * This file is part of Jerry application server.
 * Copyright (C) 2020 Sven Lukas
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

#include <jerry/config/Jerry.h>
#include <jerry/Module.h>
#include <jerry/Logger.h>
#include <jerry/URL.h>
#include <esl/Module.h>
#include <esl/module/Library.h>
#include <esl/Stacktrace.h>
#include <tinyxml2/tinyxml2.h>
#include <stdexcept>
#include <iostream>

namespace jerry {
namespace config {

namespace {
jerry::Logger logger("jerry::config::Jerry");

void throwXmlError(tinyxml2::XMLError xmlError) {
	switch(xmlError) {
	case tinyxml2::XML_SUCCESS:
		break;
	case tinyxml2::XML_NO_ATTRIBUTE:
		throw esl::addStacktrace(std::runtime_error("XML error: XML_NO_ATTRIBUTE"));
	case tinyxml2::XML_WRONG_ATTRIBUTE_TYPE:
		throw esl::addStacktrace(std::runtime_error("XML error: XML_WRONG_ATTRIBUTE_TYPE"));
	case tinyxml2::XML_ERROR_FILE_NOT_FOUND:
		throw esl::addStacktrace(std::runtime_error("XML error: XML_ERROR_FILE_NOT_FOUND"));
	case tinyxml2::XML_ERROR_FILE_COULD_NOT_BE_OPENED:
		throw esl::addStacktrace(std::runtime_error("XML error: XML_ERROR_FILE_COULD_NOT_BE_OPENED"));
	case tinyxml2::XML_ERROR_FILE_READ_ERROR:
		throw esl::addStacktrace(std::runtime_error("XML error: XML_ERROR_FILE_READ_ERROR"));
	case tinyxml2::XML_ERROR_PARSING_ELEMENT:
		throw esl::addStacktrace(std::runtime_error("XML error: XML_ERROR_PARSING_ELEMENT"));
	case tinyxml2::XML_ERROR_PARSING_ATTRIBUTE:
		throw esl::addStacktrace(std::runtime_error("XML error: XML_ERROR_PARSING_ATTRIBUTE"));
	case tinyxml2::XML_ERROR_PARSING_TEXT:
		throw esl::addStacktrace(std::runtime_error("XML error: XML_ERROR_PARSING_TEXT"));
	case tinyxml2::XML_ERROR_PARSING_CDATA:
		throw esl::addStacktrace(std::runtime_error("XML error: XML_ERROR_PARSING_CDATA"));
	case tinyxml2::XML_ERROR_PARSING_COMMENT:
		throw esl::addStacktrace(std::runtime_error("XML error: XML_ERROR_PARSING_COMMENT"));
	case tinyxml2::XML_ERROR_PARSING_DECLARATION:
		throw esl::addStacktrace(std::runtime_error("XML error: XML_ERROR_PARSING_DECLARATION"));
	case tinyxml2::XML_ERROR_PARSING_UNKNOWN:
		throw esl::addStacktrace(std::runtime_error("XML error: XML_ERROR_PARSING_UNKNOWN"));
	case tinyxml2::XML_ERROR_EMPTY_DOCUMENT:
		throw esl::addStacktrace(std::runtime_error("XML error: XML_ERROR_EMPTY_DOCUMENT"));
	case tinyxml2::XML_ERROR_MISMATCHED_ELEMENT:
		throw esl::addStacktrace(std::runtime_error("XML error: XML_ERROR_MISMATCHED_ELEMENT"));
	case tinyxml2::XML_ERROR_PARSING:
		throw esl::addStacktrace(std::runtime_error("XML error: XML_ERROR_PARSING"));
	case tinyxml2::XML_CAN_NOT_CONVERT_TEXT:
		throw esl::addStacktrace(std::runtime_error("XML error: XML_CAN_NOT_CONVERT_TEXT"));
	case tinyxml2::XML_NO_TEXT_NODE:
		throw esl::addStacktrace(std::runtime_error("XML error: XML_NO_TEXT_NODE"));
	case tinyxml2::XML_ELEMENT_DEPTH_EXCEEDED:
		throw esl::addStacktrace(std::runtime_error("XML error: XML_ELEMENT_DEPTH_EXCEEDED"));
	case tinyxml2::XML_ERROR_COUNT:
		throw esl::addStacktrace(std::runtime_error("XML error: XML_ERROR_COUNT"));
	default:
		throw esl::addStacktrace(std::runtime_error("XML error: other"));
	}
}

void loadFile(Jerry& jerry, const std::string& fileName);

void parseLibrary(std::vector<std::string>& jerryLibraries, std::vector<std::string>& eslLibraries, const tinyxml2::XMLElement& element) {
	bool hasFile = false;
	std::string file;
	std::string module;

	if(element.GetUserData() != nullptr) {
		throw esl::addStacktrace(std::runtime_error("Element has user data but it should be empty (line " + std::to_string(element.GetLineNum()) + ")"));
	}

	for(const tinyxml2::XMLAttribute* attribute = element.FirstAttribute(); attribute != nullptr; attribute = attribute->Next()) {
		if(std::string(attribute->Name()) == "file") {
			hasFile = true;
			file = attribute->Value();
		}
		else if(std::string(attribute->Name()) == "module") {
			module = attribute->Value();
		}
		else {
			throw esl::addStacktrace(std::runtime_error(std::string("Unknown attribute \"") + attribute->Name() + "\" at line " + std::to_string(element.GetLineNum())));
		}
	}

	if(hasFile == false) {
		throw esl::addStacktrace(std::runtime_error(std::string("Missing attribute \"file\" at line ") + std::to_string(element.GetLineNum())));
	}

	if(module.empty() || module == "jerry") {
		jerryLibraries.push_back(file);
	}
	else if(module == "esl") {
		eslLibraries.push_back(file);
	}
	else {
		throw esl::addStacktrace(std::runtime_error(std::string("Unknown module \"") + module + "\" at line " + std::to_string(element.GetLineNum())));
	}
}

Certificate parseCertificate(const tinyxml2::XMLElement& element) {
	Certificate certificate;
	bool hasKey = false;
	bool hasCert = false;

	if(element.GetUserData() != nullptr) {
		throw esl::addStacktrace(std::runtime_error("Element has user data but it should be empty (line " + std::to_string(element.GetLineNum()) + ")"));
	}

	for(const tinyxml2::XMLAttribute* attribute = element.FirstAttribute(); attribute != nullptr; attribute = attribute->Next()) {
		if(std::string(attribute->Name()) == "domain") {
			certificate.domain = attribute->Value();
		}
		else if(std::string(attribute->Name()) == "key") {
			hasKey = true;
			certificate.keyFile = attribute->Value();
		}
		else if(std::string(attribute->Name()) == "cert") {
			hasCert = true;
			certificate.certFile = attribute->Value();
		}
		else {
			throw esl::addStacktrace(std::runtime_error(std::string("Unknown attribute \"") + attribute->Name() + "\" at line " + std::to_string(element.GetLineNum())));
		}
	}

	if(hasKey == false) {
		throw esl::addStacktrace(std::runtime_error("Missing attribute \"key\" at line " + std::to_string(element.GetLineNum())));
	}
	if(hasCert == false) {
		throw esl::addStacktrace(std::runtime_error("Missing attribute \"cert\" at line " + std::to_string(element.GetLineNum())));
	}
	return certificate;
}

std::string parseInclude(Jerry& jerry, const tinyxml2::XMLElement& element) {
	bool hasFile = false;
	std::string fileName;

	if(element.GetUserData() != nullptr) {
		throw esl::addStacktrace(std::runtime_error("Element has user data but it should be empty (line " + std::to_string(element.GetLineNum()) + ")"));
	}

	for(const tinyxml2::XMLAttribute* attribute = element.FirstAttribute(); attribute != nullptr; attribute = attribute->Next()) {
		if(std::string(attribute->Name()) == "file") {
			hasFile = true;
			fileName = attribute->Value();
		}
		else {
			throw esl::addStacktrace(std::runtime_error(std::string("Unknown attribute \"") + attribute->Name() + "\" at line " + std::to_string(element.GetLineNum())));
		}
	}

	if(hasFile == false) {
		throw esl::addStacktrace(std::runtime_error(std::string("Missing attribute \"file\" at line ") + std::to_string(element.GetLineNum())));
	}

	loadFile(jerry, fileName);

	return fileName;
}

Parameter parseParameter(const tinyxml2::XMLElement& element) {
	Parameter parameter;
	bool hasKey = false;
	bool hasValue = false;

	if(element.GetUserData() != nullptr) {
		throw esl::addStacktrace(std::runtime_error("Element has user data but it should be empty (line " + std::to_string(element.GetLineNum()) + ")"));
	}

	for(const tinyxml2::XMLAttribute* attribute = element.FirstAttribute(); attribute != nullptr; attribute = attribute->Next()) {
		if(std::string(attribute->Name()) == "key") {
			hasKey = true;
			parameter.key = attribute->Value();
		}
		else if(std::string(attribute->Name()) == "value") {
			hasValue = true;
			parameter.value = attribute->Value();
		}
		else {
			throw esl::addStacktrace(std::runtime_error(std::string("Unknown attribute \"") + attribute->Name() + "\" at line " + std::to_string(element.GetLineNum())));
		}
	}

	if(hasKey == false) {
		throw esl::addStacktrace(std::runtime_error(std::string("Missing attribute \"key\" at line ") + std::to_string(element.GetLineNum())));
	}
	if(hasValue == false) {
		throw esl::addStacktrace(std::runtime_error(std::string("Missing attribute \"value\" at line ") + std::to_string(element.GetLineNum())));
	}
	return parameter;
}

LevelSetting parseLoggerSetting(const tinyxml2::XMLElement& element) {
	LevelSetting setting;

	bool hasClass = false;
	bool hasLevel = false;

	if(element.GetUserData() != nullptr) {
		throw esl::addStacktrace(std::runtime_error("Element has user data but it should be empty (line " + std::to_string(element.GetLineNum()) + ")"));
	}

	for(const tinyxml2::XMLAttribute* attribute = element.FirstAttribute(); attribute != nullptr; attribute = attribute->Next()) {
		if(std::string(attribute->Name()) == "class") {
			hasClass = true;
			setting.className = attribute->Value();
		}
		else if(std::string(attribute->Name()) == "level") {
			hasLevel = true;
			setting.level = attribute->Value();
		}
		else {
			throw esl::addStacktrace(std::runtime_error(std::string("Unknown attribute \"") + attribute->Name() + "\" at line " + std::to_string(element.GetLineNum())));
		}
	}

	if(hasClass == false) {
		throw esl::addStacktrace(std::runtime_error(std::string("Missing attribute \"class\" at line ") + std::to_string(element.GetLineNum())));
	}
	if(hasLevel == false) {
		throw esl::addStacktrace(std::runtime_error(std::string("Missing attribute \"level\" at line ") + std::to_string(element.GetLineNum())));
	}
	return setting;
}

Logger parseLoggerConfig(const tinyxml2::XMLElement& element) {
	Logger configLogger;
	bool hasLayout = false;

	if(element.GetUserData() != nullptr) {
		throw esl::addStacktrace(std::runtime_error("Element has user data but it should be empty (line " + std::to_string(element.GetLineNum()) + ")"));
	}
	for(const tinyxml2::XMLAttribute* attribute = element.FirstAttribute(); attribute != nullptr; attribute = attribute->Next()) {
		if(std::string(attribute->Name()) == "layout") {
			hasLayout = true;
			configLogger.layout = attribute->Value();
		}
		else {
			throw esl::addStacktrace(std::runtime_error(std::string("Unknown attribute \"") + attribute->Name() + "\" at line " + std::to_string(element.GetLineNum())));
		}
	}

	if(hasLayout == false) {
		throw esl::addStacktrace(std::runtime_error(std::string("Missing attribute \"layout\" at line ") + std::to_string(element.GetLineNum())));
	}

	for(const tinyxml2::XMLNode* node = element.FirstChild(); node != nullptr; node = node->NextSibling()) {
		const tinyxml2::XMLElement* innerElement = node->ToElement();

		if(innerElement == nullptr) {
			continue;
		}

		if(innerElement->Name() == nullptr) {
			throw esl::addStacktrace(std::runtime_error("Element name is empty at line " + std::to_string(innerElement->GetLineNum())));
		}

		std::string innerElementName(innerElement->Name());

		if(innerElementName == "layout") {
			configLogger.layoutSettings.push_back(parseParameter(*innerElement));
		}
		else if(innerElementName == "setting") {
			configLogger.levelSettings.push_back(parseLoggerSetting(*innerElement));
		}
		else {
			throw esl::addStacktrace(std::runtime_error("Unknown element name \"" + std::string(innerElement->Name()) + "\" at line " + std::to_string(innerElement->GetLineNum())));
		}
	}
	return configLogger;
}

Object parseObject(const tinyxml2::XMLElement& element) {
	Object object;
	bool hasName = false;
	bool hasImplementation = false;

	if(element.GetUserData() != nullptr) {
		throw esl::addStacktrace(std::runtime_error("Element has user data but it should be empty (line " + std::to_string(element.GetLineNum()) + ")"));
	}

	for(const tinyxml2::XMLAttribute* attribute = element.FirstAttribute(); attribute != nullptr; attribute = attribute->Next()) {
		if(std::string(attribute->Name()) == "id") {
			hasName = true;
			object.id = attribute->Value();
		}
		else if(std::string(attribute->Name()) == "implementation") {
			hasImplementation = true;
			object.implementation = attribute->Value();
		}
		else {
			throw esl::addStacktrace(std::runtime_error(std::string("Unknown attribute \"") + attribute->Name() + "\" at line " + std::to_string(element.GetLineNum())));
		}
	}

	if(hasName == false) {
		throw esl::addStacktrace(std::runtime_error(std::string("Missing attribute \"name\" at line ") + std::to_string(element.GetLineNum())));
	}
	if(hasImplementation == false) {
		throw esl::addStacktrace(std::runtime_error(std::string("Missing attribute \"implementation\" at line ") + std::to_string(element.GetLineNum())));
	}

	for(const tinyxml2::XMLNode* node = element.FirstChild(); node != nullptr; node = node->NextSibling()) {
		const tinyxml2::XMLElement* innerElement = node->ToElement();

		if(innerElement == nullptr) {
			continue;
		}

		if(innerElement->Name() == nullptr) {
			throw esl::addStacktrace(std::runtime_error("Element name is empty at line " + std::to_string(innerElement->GetLineNum())));
		}

		std::string innerElementName(innerElement->Name());

		if(innerElementName == "parameter") {
			object.parameters.push_back(parseParameter(*innerElement));
		}
		else {
			throw esl::addStacktrace(std::runtime_error("Unknown element name \"" + std::string(innerElement->Name()) + "\" at line " + std::to_string(innerElement->GetLineNum())));
		}
	}
	return object;
}

Reference parseReference(const tinyxml2::XMLElement& element) {
	Reference reference;
	bool hasId = false;
	bool hasRefName = false;

	if(element.GetUserData() != nullptr) {
		throw esl::addStacktrace(std::runtime_error("Element has user data but it should be empty (line " + std::to_string(element.GetLineNum()) + ")"));
	}

	for(const tinyxml2::XMLAttribute* attribute = element.FirstAttribute(); attribute != nullptr; attribute = attribute->Next()) {
		if(std::string(attribute->Name()) == "id") {
			hasId = true;
			reference.id = attribute->Value();
		}
		else if(std::string(attribute->Name()) == "ref-id") {
			hasRefName = true;
			reference.refId = attribute->Value();
		}
		else {
			throw esl::addStacktrace(std::runtime_error(std::string("Unknown attribute \"") + attribute->Name() + "\" at line " + std::to_string(element.GetLineNum())));
		}
	}

	if(hasId == false) {
		throw esl::addStacktrace(std::runtime_error(std::string("Missing attribute \"id\" at line ") + std::to_string(element.GetLineNum())));
	}
	if(hasRefName == false) {
		throw esl::addStacktrace(std::runtime_error(std::string("Missing attribute \"ref-id\" at line ") + std::to_string(element.GetLineNum())));
	}
	return reference;
}

std::string parseRequesthandler(const tinyxml2::XMLElement& element) {
	std::string implementation;
	bool hasImplementation = false;

	if(element.GetUserData() != nullptr) {
		throw esl::addStacktrace(std::runtime_error("Element has user data but it should be empty (line " + std::to_string(element.GetLineNum()) + ")"));
	}

	for(const tinyxml2::XMLAttribute* attribute = element.FirstAttribute(); attribute != nullptr; attribute = attribute->Next()) {
		if(std::string(attribute->Name()) == "implementation") {
			hasImplementation = true;
			implementation = attribute->Value();
		}
		else {
			throw esl::addStacktrace(std::runtime_error(std::string("Unknown attribute \"") + attribute->Name() + "\" at line " + std::to_string(element.GetLineNum())));
		}
	}

	if(hasImplementation == false) {
		throw esl::addStacktrace(std::runtime_error(std::string("Missing attribute \"implementation\" at line ") + std::to_string(element.GetLineNum())));
	}
	return implementation;
}

Listener parseListener(const tinyxml2::XMLElement& element, ListenerType listenerType) {
	Listener listener;
	bool hasURL = false;
	bool hasPath = false;

	listener.listenerType = listenerType;

	if(element.GetUserData() != nullptr) {
		throw esl::addStacktrace(std::runtime_error("Element has user data but it should be empty (line " + std::to_string(element.GetLineNum()) + ")"));
	}

	for(const tinyxml2::XMLAttribute* attribute = element.FirstAttribute(); attribute != nullptr; attribute = attribute->Next()) {
		if(listenerType == ListenerType::listener && std::string(attribute->Name()) == "url") {
			hasURL = true;
			listener.url = attribute->Value();
		}
		else if(listenerType == ListenerType::endpoint && std::string(attribute->Name()) == "path") {
			hasPath = true;
			listener.path = attribute->Value();
		}
		else {
			throw esl::addStacktrace(std::runtime_error(std::string("Unknown attribute \"") + attribute->Name() + "\" at line " + std::to_string(element.GetLineNum())));
		}
	}

	if(listenerType == ListenerType::listener && hasURL == false) {
		throw esl::addStacktrace(std::runtime_error(std::string("Missing attribute \"url\" at line ") + std::to_string(element.GetLineNum())));
	}
	if(listenerType == ListenerType::endpoint && hasPath == false) {
		throw esl::addStacktrace(std::runtime_error(std::string("Missing attribute \"path\" at line ") + std::to_string(element.GetLineNum())));
	}

	for(const tinyxml2::XMLNode* node = element.FirstChild(); node != nullptr; node = node->NextSibling()) {
		const tinyxml2::XMLElement* innerElement = node->ToElement();

		if(innerElement == nullptr) {
			continue;
		}

		if(innerElement->Name() == nullptr) {
			throw esl::addStacktrace(std::runtime_error("Element name is empty at line " + std::to_string(innerElement->GetLineNum())));
		}

		std::string innerElementName(innerElement->Name());

		if(innerElementName == "object") {
			listener.objects.push_back(parseObject(*innerElement));
		}
		else if(innerElementName == "reference") {
			listener.references.push_back(parseReference(*innerElement));
		}
		else if(innerElementName == "endpoint") {
			listener.contextEndpoints.push_back(parseListener(*innerElement, ListenerType::endpoint));
		}
		else if(innerElementName == "context") {
			listener.contextEndpoints.push_back(parseListener(*innerElement, ListenerType::context));
		}
		else if(innerElementName == "requesthandler") {
			listener.requesthandlers.push_back(parseRequesthandler(*innerElement));
		}
		else {
			throw esl::addStacktrace(std::runtime_error("Unknown element name \"" + std::string(innerElement->Name()) + "\" at line " + std::to_string(innerElement->GetLineNum())));
		}
	}
	return listener;
}

void parseJerry(Jerry& jerry, const tinyxml2::XMLElement& element) {
	if(element.GetUserData() != nullptr) {
		throw esl::addStacktrace(std::runtime_error("Node has user data but it should be empty (line " + std::to_string(element.GetLineNum()) + ")"));
	}

	for(const tinyxml2::XMLAttribute* attribute = element.FirstAttribute(); attribute != nullptr; attribute = attribute->Next()) {
		throw esl::addStacktrace(std::runtime_error(std::string("Unknown attribute \"") + attribute->Name() + "\"at line " + std::to_string(element.GetLineNum())));
	}

	for(const tinyxml2::XMLNode* node = element.FirstChild(); node != nullptr; node = node->NextSibling()) {
		const tinyxml2::XMLElement* innerElement = node->ToElement();

		if(innerElement == nullptr) {
			continue;
		}

		if(innerElement->Name() == nullptr) {
			throw esl::addStacktrace(std::runtime_error("Element name is empty at line " + std::to_string(innerElement->GetLineNum())));
		}

		std::string innerElementName(innerElement->Name());

		if(innerElementName == "library") {
			parseLibrary(jerry.libraries, jerry.eslLibraries, *innerElement);
		}
		else if(innerElementName == "certificate") {
			jerry.certificates.push_back(parseCertificate(*innerElement));
		}
		else if(innerElementName == "include") {
			jerry.includes.push_back(parseInclude(jerry, *innerElement));
		}
		else if(innerElementName == "logger") {
			jerry.loggerConfig = parseLoggerConfig(*innerElement);
		}
		else if(innerElementName == "object") {
			jerry.objects.push_back(parseObject(*innerElement));
		}
		else if(innerElementName == "listener") {
			jerry.listeners.push_back(parseListener(*innerElement, ListenerType::listener));
		}
		else {
			throw esl::addStacktrace(std::runtime_error("Unknown element name \"" + std::string(innerElement->Name()) + "\" at line " + std::to_string(innerElement->GetLineNum())));
		}
	}
}

void loadFile(Jerry& jerry, const std::string& fileName) {
	if(jerry.filesLoaded.find(fileName) != std::end(jerry.filesLoaded)) {
		return;
	}
	jerry.filesLoaded.insert(fileName);

	tinyxml2::XMLDocument doc;
	throwXmlError(doc.LoadFile(fileName.c_str()));

	tinyxml2::XMLElement* element = doc.RootElement();
	if(element == nullptr) {
		throw esl::addStacktrace(std::runtime_error("XML document without root element"));
	}
	if(element->Name() == nullptr) {
		throw esl::addStacktrace(std::runtime_error("Name of XML root element is empty"));
	}
	if(std::string(element->Name()) != "jerry") {
		throw esl::addStacktrace(std::runtime_error("Name of XML root element is \"" + std::string(element->Name()) + "\" but should be \"jerry\""));
	}
	jerry.fileName = fileName;
	parseJerry(jerry, *element);
}

std::string space(unsigned int i) {
	std::string s;
	for(; i>0; --i) {
		s += " ";
	}
	return s;
}

void printObjects(unsigned int spaces, const std::vector<Object>& objects) {
	for(const auto& entry : objects) {
		std::cout << space(spaces) << "<object id=\"" << entry.id << "\" implementation=\"" << entry.implementation << "\">\n";
		for(const auto& entry2 : entry.parameters) {
			std::cout << space(spaces+2) << "<parameter key=\"" << entry2.key << "\" value=\"" << entry2.value << "\"/>\n";
		}
		std::cout << space(spaces) << "<object/>\n";
	}
}

void printListeners(unsigned int spaces, const std::vector<Listener>& listeners) {
	for(const auto& entry : listeners) {
		switch(entry.listenerType) {
		case ListenerType::listener:
			std::cout << space(spaces) << "<listener url=\"" << entry.url << "\">\n";
			break;
		case ListenerType::endpoint:
			std::cout << space(spaces) << "<endpoint path=\"" << entry.path << "\">\n";
			break;
		case ListenerType::context:
			std::cout << space(spaces) << "<context>\n";
			break;
		}

		printObjects(spaces+2, entry.objects);

		for(const auto& entry2 : entry.references) {
			std::cout << space(spaces+2) << "<reference id=\"" << entry2.id << "\" ref-id=\"" << entry2.refId << "\"/>\n";
		}

		printListeners(spaces+2, entry.contextEndpoints);

		switch(entry.listenerType) {
		case ListenerType::listener:
			std::cout << space(spaces) << "<listener/>\n";
			break;
		case ListenerType::endpoint:
			std::cout << space(spaces) << "<endpoint/>\n";
			break;
		case ListenerType::context:
			std::cout << space(spaces) << "<context/>\n";
			break;
		}
	}
}

} /* anonymous namespace */

Jerry::Jerry(const std::string& aFileName)
: fileName(aFileName)
{
	loadFile(*this, fileName);
}

void Jerry::loadLibraries() {
	/* load libraries and show registered modules */
	for(const auto& eslLibrary : eslLibraries) {
		esl::module::Library* lib(new esl::module::Library(eslLibrary));
		esl::getModule().addModule(lib->getModule());
	}
	for(const auto& jerryLibrary : libraries) {
		esl::module::Library* lib(new esl::module::Library(jerryLibrary));
		jerry::getModule().addModule(lib->getModule());
	}
}

std::unique_ptr<esl::logging::Layout> Jerry::getLayout() const {
	std::unique_ptr<esl::logging::Layout> layout(new esl::logging::Layout(loggerConfig.layout));

	for(auto const setting : loggerConfig.layoutSettings) {
		layout->setParameter(setting.key, setting.value);
	}
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

	return layout;
}

void Jerry::setEngine(engine::Engine& engine) const {
	for(const auto& configCertificate : certificates) {
		engine.addCertificate(configCertificate.domain, configCertificate.keyFile, configCertificate.certFile);
	}

	for(const auto& configObject : objects) {
		esl::object::parameter::Interface::Object& parameterObject = engine.addObject(configObject.id, configObject.implementation);
		for(const auto& parameterConfig : configObject.parameters) {
			parameterObject.setParameter(parameterConfig.key, parameterConfig.value);
		}
	}

	for(const auto& configListener : listeners) {
		engine::Listener& engineListener = engine.addListener(URL(configListener.url));
		setEngineListener(engineListener, configListener);
	}
}

void Jerry::print() {
	std::cout << "\n<jerry>\n";
	for(const auto& entry : libraries) {
		std::cout << "  <library file=\"" << entry << "\"/>\n";
	}
	for(const auto& entry : includes) {
		std::cout << "  <include file=\"" << entry << "\"/>\n";
	}
	for(const auto& entry : certificates) {
		std::cout << "  <certificate domain=\"" << entry.domain << "\" key=\"" << entry.keyFile << "\" cert=\"" << entry.certFile << "\"/>\n";
	}

	std::cout << "  <logger layout=\"" << loggerConfig.layout << "\">\n";
	for(const auto& entry : loggerConfig.levelSettings) {
		std::cout << "    <setting class=\"" << entry.className << "\" level=\"" << entry.level << "\"/>\n";
	}
	for(const auto& entry : loggerConfig.layoutSettings) {
		std::cout << "    <layout key=\"" << entry.key << "\" value=\"" << entry.value << "\"/>\n";
	}
	std::cout << "  </logger/>\n";

	printObjects(2, objects);
	printListeners(2, listeners);

	std::cout << "</jerry>\n";
}

void Jerry::setEngineContext(engine::Context& engineContext, const Listener& configListener) const {
	for(const auto& object : configListener.objects) {
		esl::object::parameter::Interface::Object& engineObject = engineContext.addObject(object.id, object.implementation);
		for(const auto& parameter : object.parameters) {
			engineObject.setParameter(parameter.key, parameter.value);
		}
	}

	for(const auto& reference : configListener.references) {
		engineContext.addReference(reference.id, reference.refId);
	}

	for(const auto& configRequesthandler : configListener.requesthandlers) {
		engineContext.addRequestHandler(configRequesthandler);
	}

	for(const auto& configContext : configListener.contextEndpoints) {
		if(configContext.listenerType != ListenerType::context) {
			continue;
		}

		engine::Context& newEngineContext = engineContext.addContext();
		setEngineContext(newEngineContext, configContext);
	}

}

void Jerry::setEngineEndpoint(engine::Endpoint& engineEndpoint, const Listener& configListener) const {
	setEngineContext(engineEndpoint, configListener);

	for(const auto& configEndpoint : configListener.contextEndpoints) {
		if(configEndpoint.listenerType != ListenerType::endpoint) {
			continue;
		}

		engine::Endpoint& newEngineEndpoint = engineEndpoint.addEndpoint(configEndpoint.path);
		setEngineEndpoint(newEngineEndpoint, configEndpoint);
	}
}

void Jerry::setEngineListener(engine::Listener& engineListener, const Listener& configListener) const {
	setEngineEndpoint(engineListener, configListener);
}



} /* namespace config */
} /* namespace jerry */
