/*
 * This file is part of Jerry application server.
 * Copyright (C) 2020-2021 Sven Lukas
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

#include <jerry/config/http/Context.h>
#include <jerry/config/http/Endpoint.h>
#include <jerry/config/http/RequestHandler.h>
#include <jerry/config/http/Entry.h>
#include <jerry/config/Object.h>
#include <jerry/config/XMLException.h>

#include <esl/utility/String.h>

namespace jerry {
namespace config {
namespace http {

Context::Context(const std::string& fileName, const tinyxml2::XMLElement& element, bool aIsGlobal)
: Config(fileName, element),
  isGlobal(aIsGlobal)
{
	if(element.GetUserData() != nullptr) {
		throw XMLException(*this, "Element has user data but it should be empty");
	}

	bool hasInherit = false;

	for(const tinyxml2::XMLAttribute* attribute = element.FirstAttribute(); attribute != nullptr; attribute = attribute->Next()) {
		if(std::string(attribute->Name()) == "id") { // && isGlobal == true) {
			id = attribute->Value();
			if(id == "") {
				throw XMLException(*this, "Invalid value \"\" for attribute 'id'");
			}
			if(refId != "") {
				throw XMLException(*this, "Attribute 'id' is not allowed together with attribute 'ref-id'.");
			}
		}
		else if(std::string(attribute->Name()) == "ref-id") {
			refId = attribute->Value();
			if(isGlobal) {
				throw XMLException(*this, "Attribute 'ref-id' is not allowed in global scope.");
			}
			if(refId == "") {
				throw XMLException(*this, "Invalid value \"\" for attribute 'ref-id'");
			}
			if(id != "") {
				throw XMLException(*this, "Attribute 'ref-id' is not allowed together with attribute 'id'.");
			}
			if(hasInherit) {
				throw XMLException(*this, "Attribute 'ref-id' is not allowed together with attribute 'inherit'.");
			}
		}
		else if(std::string(attribute->Name()) == "inherit") {
			std::string inheritStr = esl::utility::String::toLower(attribute->Value());
			hasInherit = true;
			if(inheritStr == "true") {
				inherit = true;
			}
			else if(inheritStr == "false") {
				inherit = false;
			}
			else {
				throw XMLException(*this, "Invalid value \"" + std::string(attribute->Value()) + "\" for attribute 'inherit'");
			}
			if(refId != "") {
				throw XMLException(*this, "Attribute 'inherit' is not allowed together with attribute 'ref-id'.");
			}
		}
		else {
			throw XMLException(*this, "Unknown attribute '" + std::string(attribute->Name()) + "'");
		}
	}

	if(isGlobal && id == "") {
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

void Context::save(std::ostream& oStream, std::size_t spaces) const {
	if(isGlobal) {
		oStream << makeSpaces(spaces) << "<http-context";
	}
	else {
		oStream << makeSpaces(spaces) << "<context";
	}

	if(refId != "") {
		oStream << " ref-id=\"" << refId << "\"/>\n";
	}
	else {
		if(id != "") {
			oStream << " id=\"" << id << "\"";
		}
		if(inherit) {
			oStream << " inherit=\"true\"";
		}
		else {
			oStream << " inherit=\"false\"";
		}
		oStream << ">\n";

		for(const auto& entry : entries) {
			entry.save(oStream, spaces+2);
		}

		for(const auto& entry : responseHeaders) {
			entry.saveResponseHeader(oStream, spaces+2);
		}

		exceptions.save(oStream, spaces+2);

		if(isGlobal) {
			oStream << makeSpaces(spaces) << "</http-context>\n";
		}
		else {
			oStream << makeSpaces(spaces) << "</context>\n";
		}
	}
}

void Context::install(engine::Engine& engine) const {
	engine::http::server::Context* contextPtr;
	{
		std::unique_ptr<engine::http::server::Context> context(new engine::http::server::Context);
		contextPtr = context.get();
		engine.addObject(id, std::unique_ptr<esl::object::Interface::Object>(context.release()));
	}

	if(inherit) {
		contextPtr->ObjectContext::setParent(&engine);
	}

	/* *****************
	 * install entries *
	 * *****************/
	for(const auto& entry : entries) {
		entry.install(*contextPtr);
	}

	/* **********************
	 * Set response headers *
	 * **********************/
	for(const auto& responseHeader : responseHeaders) {
		contextPtr->addHeader(responseHeader.key, responseHeader.value);
	}

	exceptions.install(*contextPtr);
}

void Context::install(engine::http::server::Context& engineHttpContext) const {
	if(refId == "") {
		engine::http::server::Context& newEngineHttpContext = engineHttpContext.addContext(id, inherit);

		/* *****************
		 * install entries *
		 * *****************/
		for(const auto& entry : entries) {
			entry.install(newEngineHttpContext);
		}

		/* **********************
		 * Set response headers *
		 * **********************/
		for(const auto& responseHeader : responseHeaders) {
			newEngineHttpContext.addHeader(responseHeader.key, responseHeader.value);
		}

		exceptions.install(newEngineHttpContext);
	}
	else {
		engineHttpContext.addContext(refId);
	}
}

void Context::parseInnerElement(const tinyxml2::XMLElement& element) {

	if(refId != "") {
		throw XMLException(*this, "No content allowed if 'ref-id' is specified.");
	}

	std::string innerElementName(element.Name());

	if(innerElementName == "response-header") {
		responseHeaders.push_back(Setting(getFileName(), element, false));
	}
	else if(innerElementName == "exceptions") {
		exceptions = Exceptions(getFileName(), element);
	}
	else {
		entries.push_back(Entry(getFileName(), element));
	}
}

} /* namespace http */
} /* namespace config */
} /* namespace jerry */
