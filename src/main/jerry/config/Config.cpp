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

#include <jerry/config/Config.h>
#include <jerry/config/RequestHandler.h>
#include <jerry/config/Context.h>
#include <jerry/config/Endpoint.h>
#include <jerry/Module.h>
#include <jerry/Logger.h>
#include <jerry/utility/URL.h>

#include <esl/object/Settings.h>
#include <esl/object/ValueSettings.h>
#include <esl/Module.h>
#include <esl/Stacktrace.h>

#include <stdexcept>
#include <iostream>

namespace jerry {
namespace config {

namespace {
jerry::Logger logger("jerry::config::Config");

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

std::string space(unsigned int i) {
	std::string s;
	for(; i>0; --i) {
		s += " ";
	}
	return s;
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

void Config::loadFile(const std::string& fileName) {
	if(filesLoaded.find(fileName) != std::end(filesLoaded)) {
		return;
	}
	filesLoaded.insert(fileName);

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
	if(element->GetUserData() != nullptr) {
		throw esl::addStacktrace(std::runtime_error("Node has user data but it should be empty (line " + std::to_string(element->GetLineNum()) + ")"));
	}

	for(const tinyxml2::XMLAttribute* attribute = element->FirstAttribute(); attribute != nullptr; attribute = attribute->Next()) {
		throw esl::addStacktrace(std::runtime_error(std::string("Unknown attribute \"") + attribute->Name() + "\" at line " + std::to_string(element->GetLineNum())));
	}

	for(const tinyxml2::XMLNode* node = element->FirstChild(); node != nullptr; node = node->NextSibling()) {
		const tinyxml2::XMLElement* innerElement = node->ToElement();

		if(innerElement == nullptr) {
			continue;
		}

		if(innerElement->Name() == nullptr) {
			throw esl::addStacktrace(std::runtime_error("Element name is empty at line " + std::to_string(innerElement->GetLineNum())));
		}

		std::string innerElementName(innerElement->Name());

		if(innerElementName == "include") {
			parseInclude(*innerElement);
		}
		else if(innerElementName == "library") {
			parseLibrary(*innerElement);
		}
		else if(innerElementName == "certificate") {
			certificates.push_back(Certificate(*innerElement));
		}
		else if(innerElementName == "logger") {
			loggerConfig = LoggerConfig(*innerElement);
		}
		else if(innerElementName == "object") {
			objects.push_back(Object(*innerElement));
		}
		else if(innerElementName == "listener") {
			listeners.push_back(Listener(*innerElement));
		}
		else {
			throw esl::addStacktrace(std::runtime_error("Unknown element name \"" + std::string(innerElement->Name()) + "\" at line " + std::to_string(innerElement->GetLineNum())));
		}
	}
}

void Config::loadLibraries() {
	/* load and add libraries */
	for(auto& eslLibrary : eslLibraries) {
		if(eslLibrary.second) {
			throw esl::addStacktrace(std::runtime_error(std::string("Library \"") + eslLibrary.first + "\" loaded already."));
		}
		eslLibrary.second = new esl::module::Library(eslLibrary.first);
		esl::module::Module& aLibModule = eslLibrary.second->getModule("");
		esl::getModule().addInterfaces(aLibModule);
	}
	for(auto& jerryLibrary : libraries) {
		if(jerryLibrary.second) {
			throw esl::addStacktrace(std::runtime_error(std::string("Library \"") + jerryLibrary.first + "\" loaded already."));
		}
		jerryLibrary.second = new esl::module::Library(jerryLibrary.first);
		esl::module::Module& aLibModule = jerryLibrary.second->getModule("");
		jerry::getModule().addInterfaces(aLibModule);
	}


	/* add and replace esl interfaces to loaded libraries by own esl libraries */
	for(auto& eslLibrary : eslLibraries) {
		if(!eslLibrary.second) {
			throw esl::addStacktrace(std::runtime_error(std::string("Library \"") + eslLibrary.first + "\" not loaded."));
		}
		esl::module::Module* aLibEslModule = eslLibrary.second->getModulePointer("esl");
		if(aLibEslModule) {
			aLibEslModule->replaceInterfaces(esl::getModule());
		}
	}

	for(auto& jerryLibrary : libraries) {
		if(!jerryLibrary.second) {
			throw esl::addStacktrace(std::runtime_error(std::string("Library \"") + jerryLibrary.first + "\" not loaded."));
		}
		esl::module::Module* aLibEslModule = jerryLibrary.second->getModulePointer("esl");
		if(aLibEslModule) {
			aLibEslModule->replaceInterfaces(esl::getModule());
		}
	}
}

std::unique_ptr<esl::logging::Layout> Config::createLayout() const {
#if 1
	esl::object::ValueSettings settings;
	for(auto const setting : loggerConfig.layoutSettings) {
		settings.addSetting(setting.key, setting.value);
	}

	return std::unique_ptr<esl::logging::Layout>(new esl::logging::Layout(settings, loggerConfig.layout));
#else
	std::unique_ptr<esl::logging::Layout> layout(new esl::logging::Layout(loggerConfig.layout));

	for(auto const setting : loggerConfig.layoutSettings) {
		layout->addSetting(setting.key, setting.value);
	}

	return layout;
#endif
}

void Config::setLogLevel() const {
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
}

void Config::setEngine(engine::Engine& engine) const {
	for(const auto& configCertificate : certificates) {
		engine.addCertificate(configCertificate.domain, configCertificate.keyFile, configCertificate.certFile);
	}

	setEngineContextObject(engine, objects);

	for(const auto& configListener : listeners) {
		engine::Listener& engineListener = engine.addListener(utility::URL(configListener.url));
		setEngineListener(engineListener, configListener);
	}
}

void Config::save(std::ostream& oStream) const {
	oStream << "\n<jerry>\n";
	for(const auto& entry : eslLibraries) {
		oStream << "  <library module=\"esl\" file=\"" << entry.first << "\"/>\n";
	}
	for(const auto& entry : libraries) {
		oStream << "  <library file=\"" << entry.first << "\"/>\n";
	}
	for(const auto& entry : includes) {
		oStream << "  <include file=\"" << entry << "\"/>\n";
	}
	for(const auto& entry : certificates) {
		entry.save(oStream, 2);
	}

	loggerConfig.save(oStream, 2);

	for(const auto& entry : objects) {
		entry.save(oStream, 2);
	}

	for(const Listener& listener : listeners) {
		listener.save(oStream, 2);
	}

	oStream << "</jerry>\n";
}


void Config::parseInclude(const tinyxml2::XMLElement& element) {
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

	loadFile(fileName);
	includes.push_back(fileName);
}

void Config::parseLibrary(const tinyxml2::XMLElement& element) {
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
		libraries.push_back(std::make_pair(file, nullptr));
	}
	else if(module == "esl") {
		eslLibraries.push_back(std::make_pair(file, nullptr));
	}
	else {
		throw esl::addStacktrace(std::runtime_error(std::string("Unknown module \"") + module + "\" at line " + std::to_string(element.GetLineNum())));
	}
}

void Config::setEngineContextObject(engine::BaseContext& engineContext, const std::vector<Object>& objects) const {
	for(const auto& object : objects) {
		esl::object::Interface::Object& engineObject = engineContext.addObject(object.id, object.implementation);
		addSettings(engineObject, object.implementation, object.settings);
	}
}

void Config::setEngineContextReferences(engine::Context& engineContext, const std::vector<Reference>& references) const {
	for(const auto& reference : references) {
		engineContext.addReference(reference.id, reference.refId);
	}
}

void Config::setEngineContextEntries(engine::Context& engineContext, const std::vector<Entry>& entries) const {
	for(const auto& entry : entries) {
		if(entry.getRequestHandler()) {
			if(entry.getRequestHandler()->refId || entry.getRequestHandler()->objectImplementation || entry.getRequestHandler()->settings.size() > 0) {
				engine::Context& newEngineContext = engineContext.addContext();

				if(entry.getRequestHandler()->refId) {
					if(entry.getRequestHandler()->objectImplementation || entry.getRequestHandler()->settings.size() > 0) {
						throw esl::addStacktrace(std::runtime_error("definition of refId in requestHandler does not allow simultaneously definition of object-implementation or adding parameters"));
					}
					newEngineContext.addReference("", *entry.getRequestHandler()->refId);
				}
				else {
					std::string objectImplementation;
					if(entry.getRequestHandler()->objectImplementation) {
						objectImplementation = *entry.getRequestHandler()->objectImplementation;
					}
					else {
						objectImplementation = entry.getRequestHandler()->implementation;
					}
					esl::object::Interface::Object& engineObject = newEngineContext.addObject("", objectImplementation);
					addSettings(engineObject, objectImplementation, entry.getRequestHandler()->settings);
				}

				newEngineContext.addRequestHandler(entry.getRequestHandler()->implementation);
			}
			else {
				engineContext.addRequestHandler(entry.getRequestHandler()->implementation);
			}
		}

		if(entry.getContext()) {
			engine::Context& newEngineContext = engineContext.addContext();
			setEngineContext(newEngineContext, *entry.getContext());
		}

		if(entry.getEndpoint()) {
			engine::Endpoint& newEngineEndpoint = engineContext.addEndpoint(entry.getEndpoint()->path);
			setEngineEndpoint(newEngineEndpoint, *entry.getEndpoint());
		}
	}
}

void Config::setEngineEndpointResponseHeaders(engine::Endpoint& engineEndpoint, const std::vector<Setting>& responseHeaders) const {
	for(const auto& responseHeader : responseHeaders) {
		engineEndpoint.addHeader(responseHeader.key, responseHeader.value);
	}
}

void Config::setEngineEndpointExceptions(engine::Endpoint& engineEndpoint, const Exceptions& exceptions) const {
	/* set showExceptions */
	if(exceptions.showExceptions == OptionalBool::obTrue) {
		engineEndpoint.setShowException(true);
	}
	else if(exceptions.showExceptions == OptionalBool::obFalse) {
		engineEndpoint.setShowException(false);
	}

	/* set showStacktrace */
	if(exceptions.showStacktrace == OptionalBool::obTrue) {
		engineEndpoint.setShowStacktrace(true);
	}
	else if(exceptions.showStacktrace == OptionalBool::obFalse) {
		engineEndpoint.setShowStacktrace(false);
	}

	engineEndpoint.setInheritErrorDocuments(exceptions.inheritDocuments);

	for(const auto& exceptionDocument : exceptions.documents) {
		engineEndpoint.addErrorDocument(exceptionDocument.statusCode, exceptionDocument.path, exceptionDocument.parser);
	}
}

void Config::setEngineContext(engine::Context& engineContext, const Context& configContext) const {
	setEngineContextObject(engineContext, configContext.objects);
	setEngineContextReferences(engineContext, configContext.references);
	setEngineContextEntries(engineContext, configContext.entries);
}

void Config::setEngineEndpoint(engine::Endpoint& engineEndpoint, const Endpoint& configEndpoint) const {
	setEngineContextObject(engineEndpoint, configEndpoint.objects);
	setEngineContextReferences(engineEndpoint, configEndpoint.references);
	setEngineContextEntries(engineEndpoint, configEndpoint.entries);

	setEngineEndpointResponseHeaders(engineEndpoint, configEndpoint.responseHeaders);

	setEngineEndpointExceptions(engineEndpoint, configEndpoint.exceptions);
}

void Config::setEngineListener(engine::Listener& engineListener, const Listener& configListener) const {
	setEngineContextObject(engineListener, configListener.objects);
	setEngineContextReferences(engineListener, configListener.references);
	setEngineContextEntries(engineListener, configListener.entries);

	setEngineEndpointResponseHeaders(engineListener, configListener.responseHeaders);

	setEngineEndpointExceptions(engineListener, configListener.exceptions);
}

} /* namespace config */
} /* namespace jerry */
