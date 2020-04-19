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
#include <jerry/utility/URL.h>
#include <esl/object/Settings.h>
#include <esl/Module.h>
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

void parseLibrary(std::vector<std::pair<std::string, esl::module::Library*>>& jerryLibraries, std::vector<std::pair<std::string, esl::module::Library*>>& eslLibraries, const tinyxml2::XMLElement& element) {
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
		jerryLibraries.push_back(std::make_pair(file, nullptr));
	}
	else if(module == "esl") {
		eslLibraries.push_back(std::make_pair(file, nullptr));
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

Setting parseParameter(const tinyxml2::XMLElement& element) {
	Setting setting;
	bool hasKey = false;
	bool hasValue = false;

	if(element.GetUserData() != nullptr) {
		throw esl::addStacktrace(std::runtime_error("Element has user data but it should be empty (line " + std::to_string(element.GetLineNum()) + ")"));
	}

	for(const tinyxml2::XMLAttribute* attribute = element.FirstAttribute(); attribute != nullptr; attribute = attribute->Next()) {
		if(std::string(attribute->Name()) == "key") {
			hasKey = true;
			setting.key = attribute->Value();
		}
		else if(std::string(attribute->Name()) == "value") {
			hasValue = true;
			setting.value = attribute->Value();
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
	return setting;
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
			object.settings.push_back(parseParameter(*innerElement));
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

RequestHandler parseRequesthandler(const tinyxml2::XMLElement& element) {
	RequestHandler requestHandler;
	bool hasImplementation = false;

	bool hasObjectImplementation = false;
	bool hasRefId = false;

	if(element.GetUserData() != nullptr) {
		throw esl::addStacktrace(std::runtime_error("Element has user data but it should be empty (line " + std::to_string(element.GetLineNum()) + ")"));
	}

	for(const tinyxml2::XMLAttribute* attribute = element.FirstAttribute(); attribute != nullptr; attribute = attribute->Next()) {
		if(std::string(attribute->Name()) == "implementation") {
			hasImplementation = true;
			requestHandler.implementation = attribute->Value();
		}
		else if(std::string(attribute->Name()) == "object-implementation") {
			hasObjectImplementation = true;
			requestHandler.objectImplementation = attribute->Value();
		}
		else if(std::string(attribute->Name()) == "ref-id") {
			hasRefId = true;
			requestHandler.refId = attribute->Value();
		}
		else {
			throw esl::addStacktrace(std::runtime_error(std::string("Unknown attribute \"") + attribute->Name() + "\" at line " + std::to_string(element.GetLineNum())));
		}
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
			requestHandler.settings.push_back(parseParameter(*innerElement));
		}
		else {
			throw esl::addStacktrace(std::runtime_error("Unknown element name \"" + std::string(innerElement->Name()) + "\" at line " + std::to_string(innerElement->GetLineNum())));
		}
	}

	if(hasImplementation == false) {
		throw esl::addStacktrace(std::runtime_error(std::string("Missing attribute \"implementation\" at line ") + std::to_string(element.GetLineNum())));
	}
	if(hasRefId == true && (hasObjectImplementation == true || requestHandler.settings.size() > 0)) {
		throw esl::addStacktrace(std::runtime_error(std::string("Invalid definition of request handler at line ") + std::to_string(element.GetLineNum())));
	}


	return requestHandler;
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
			std::unique_ptr<Listener> endpoint(new Listener(parseListener(*innerElement, ListenerType::endpoint)));
			listener.entries.push_back(std::make_tuple(nullptr, std::move(endpoint)));
		}
		else if(innerElementName == "context") {
			std::unique_ptr<Listener> context(new Listener(parseListener(*innerElement, ListenerType::context)));
			listener.entries.push_back(std::make_tuple(nullptr, std::move(context)));
		}
		else if(innerElementName == "requesthandler") {
			std::unique_ptr<RequestHandler> requestHandler(new RequestHandler(parseRequesthandler(*innerElement)));
			listener.entries.push_back(std::make_tuple(std::move(requestHandler), nullptr));
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
		for(const auto& entry2 : entry.settings) {
			std::cout << space(spaces+2) << "<parameter key=\"" << entry2.key << "\" value=\"" << entry2.value << "\"/>\n";
		}
		std::cout << space(spaces) << "<object/>\n";
	}
}

void printEntries(unsigned int spaces, const std::vector<std::tuple<std::unique_ptr<RequestHandler>, std::unique_ptr<Listener>>>& entries);

void printListener(unsigned int spaces, const Listener& listener) {
	switch(listener.listenerType) {
	case ListenerType::listener:
		std::cout << space(spaces) << "<listener url=\"" << listener.url << "\">\n";
		break;
	case ListenerType::endpoint:
		std::cout << space(spaces) << "<endpoint path=\"" << listener.path << "\">\n";
		break;
	case ListenerType::context:
		std::cout << space(spaces) << "<context>\n";
		break;
	default:
		break;
	}

	printObjects(spaces+2, listener.objects);

	for(const auto& entry2 : listener.references) {
		std::cout << space(spaces+2) << "<reference id=\"" << entry2.id << "\" ref-id=\"" << entry2.refId << "\"/>\n";
	}

	printEntries(spaces+2, listener.entries);

	switch(listener.listenerType) {
	case ListenerType::listener:
		std::cout << space(spaces) << "<listener/>\n";
		break;
	case ListenerType::endpoint:
		std::cout << space(spaces) << "<endpoint/>\n";
		break;
	case ListenerType::context:
		std::cout << space(spaces) << "<context/>\n";
		break;
	default:
		break;
	}
}

void printEntries(unsigned int spaces, const std::vector<std::tuple<std::unique_ptr<RequestHandler>, std::unique_ptr<Listener>>>& entries) {
	for(const auto& entry : entries) {
		if(std::get<0>(entry)) {

		}

		if(std::get<1>(entry)) {
			printListener(spaces, *std::get<1>(entry));
		}
	}
}

void addSettings(esl::object::Interface::Object& object, const std::string& implementation, const std::vector<Setting>& settings) {
	if(settings.empty()) {
		return;
	}

	esl::object::Settings* settingsObject = dynamic_cast<esl::object::Settings*>(&object);
	if(settingsObject == nullptr) {
		throw esl::addStacktrace(std::runtime_error("Cannot add settings to simple object implementation \"" + implementation + "\""));
	}
	for(const auto& setting : settings) {
		settingsObject->addSetting(setting.key, setting.value);
	}
}

} /* anonymous namespace */

Jerry::Jerry(const std::string& aFileName)
: fileName(aFileName)
{
	loadFile(*this, fileName);
}

void Jerry::loadLibraries() {
	esl::module::Module& aEslModule = esl::getModule();

	/* load and add libraries */
	for(auto& eslLibrary : eslLibraries) {
		if(eslLibrary.second) {
			throw esl::addStacktrace(std::runtime_error(std::string("Library \"") + eslLibrary.first + "\" loaded already."));
		}
		eslLibrary.second = new esl::module::Library(eslLibrary.first);
		esl::module::Module& aLibModule = eslLibrary.second->getModule("");
		aEslModule.addModule(aLibModule);
	}
	for(auto& jerryLibrary : libraries) {
		if(jerryLibrary.second) {
			throw esl::addStacktrace(std::runtime_error(std::string("Library \"") + jerryLibrary.first + "\" loaded already."));
		}
		jerryLibrary.second = new esl::module::Library(jerryLibrary.first);
		esl::module::Module& aLibModule = jerryLibrary.second->getModule("");
		jerry::getModule().addModule(aLibModule);
	}


	/* add and replace esl interfaces to loaded libraries by own esl libraries */
	for(auto& eslLibrary : eslLibraries) {
		if(!eslLibrary.second) {
			throw esl::addStacktrace(std::runtime_error(std::string("Library \"") + eslLibrary.first + "\" not loaded."));
		}
		esl::module::Module* aLibEslModule = eslLibrary.second->getModulePointer("esl");
		if(aLibEslModule) {
			aLibEslModule->replaceModule(aEslModule);
		}
	}

	for(auto& jerryLibrary : libraries) {
		if(!jerryLibrary.second) {
			throw esl::addStacktrace(std::runtime_error(std::string("Library \"") + jerryLibrary.first + "\" not loaded."));
		}
		esl::module::Module* aLibEslModule = jerryLibrary.second->getModulePointer("esl");
		if(aLibEslModule) {
			aLibEslModule->replaceModule(aEslModule);
		}
	}
}

std::unique_ptr<esl::logging::Layout> Jerry::getLayout() const {
	std::unique_ptr<esl::logging::Layout> layout(new esl::logging::Layout(loggerConfig.layout));

	for(auto const setting : loggerConfig.layoutSettings) {
		layout->addSetting(setting.key, setting.value);
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
		esl::object::Interface::Object& object = engine.addObject(configObject.id, configObject.implementation);
		addSettings(object, configObject.implementation, configObject.settings);
	}

	for(const auto& configListener : listeners) {
		engine::Listener& engineListener = engine.addListener(utility::URL(configListener.url));
		setEngineListener(engineListener, configListener);
	}
}

void Jerry::print() {
	std::cout << "\n<jerry>\n";
	for(const auto& entry : eslLibraries) {
		std::cout << "  <library module=\"esl\" file=\"" << entry.first << "\"/>\n";
	}
	for(const auto& entry : libraries) {
		std::cout << "  <library file=\"" << entry.first << "\"/>\n";
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
	for(const Listener& listener : listeners) {
		printListener(2, listener);
	}

	std::cout << "</jerry>\n";
}

void Jerry::setEngineContext(engine::Context& engineContext, const Listener& configListener) const {
	for(const auto& object : configListener.objects) {
		esl::object::Interface::Object& engineObject = engineContext.addObject(object.id, object.implementation);
		addSettings(engineObject, object.implementation, object.settings);
	}

	for(const auto& reference : configListener.references) {
		engineContext.addReference(reference.id, reference.refId);
	}

	for(const auto& entry : configListener.entries) {
		if(std::get<0>(entry)) {
			if(std::get<0>(entry)->refId.size() > 0 || std::get<0>(entry)->objectImplementation.size() > 0 || std::get<0>(entry)->settings.size() > 0) {
				engine::Context& newEngineContext = engineContext.addContext();

				if(std::get<0>(entry)->refId.size()) {
					if(std::get<0>(entry)->objectImplementation.size() > 0 || std::get<0>(entry)->settings.size() > 0) {
						throw esl::addStacktrace(std::runtime_error("definition of refId in requestHandler does not allow simultaneously definition of object-implementation or adding parameters"));
					}
					newEngineContext.addReference("", std::get<0>(entry)->refId);
				}
				else {
					std::string objectImplementation;
					if(std::get<0>(entry)->objectImplementation.size() > 0) {
						objectImplementation = std::get<0>(entry)->objectImplementation;
					}
					else {
						objectImplementation = std::get<0>(entry)->implementation;
					}
					esl::object::Interface::Object& engineObject = newEngineContext.addObject("", objectImplementation);
					addSettings(engineObject, objectImplementation, std::get<0>(entry)->settings);
				}

				newEngineContext.addRequestHandler(std::get<0>(entry)->implementation);
			}
			else {
				engineContext.addRequestHandler(std::get<0>(entry)->implementation);
			}
		}

		if(std::get<1>(entry)) {
			if(std::get<1>(entry)->listenerType == ListenerType::context) {
				engine::Context& newEngineContext = engineContext.addContext();
				setEngineContext(newEngineContext, *std::get<1>(entry));
			}
			if(std::get<1>(entry)->listenerType == ListenerType::endpoint) {
				engine::Endpoint& newEngineEndpoint = engineContext.addEndpoint(std::get<1>(entry)->path);
				setEngineContext(newEngineEndpoint, *std::get<1>(entry));
			}
		}
	}
}

void Jerry::setEngineListener(engine::Listener& engineListener, const Listener& configListener) const {
	setEngineContext(engineListener, configListener);
}



} /* namespace config */
} /* namespace jerry */
