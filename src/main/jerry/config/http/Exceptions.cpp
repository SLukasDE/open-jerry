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

#include <jerry/config/http/Exceptions.h>
#include <jerry/config/XMLException.h>

#include <esl/Stacktrace.h>

#include <stdexcept>

namespace jerry {
namespace config {
namespace http {

Exceptions::Exceptions()
: Config("")
{ }

Exceptions::Exceptions(const std::string& fileName, const tinyxml2::XMLElement& element)
: Config(fileName, element)
{
	if(element.GetUserData() != nullptr) {
		throw XMLException(*this, "Element has user data but it should be empty");
	}

	for(const tinyxml2::XMLAttribute* attribute = element.FirstAttribute(); attribute != nullptr; attribute = attribute->Next()) {
		std::string attributeName(attribute->Name());
		std::string attributeValue(attribute->Value());

		if(attributeName == "inheritDocuments") {
			if(stringToBool(inheritDocuments, attributeValue) == false) {
				throw XMLException(*this, "Unknown value \"" + attributeValue + "\" for attribute '" + attributeName + "'");
			}
		}
		else if(attributeName == "showExceptions") {
			bool b;
			if(stringToBool(b, attributeValue) == false) {
				throw XMLException(*this, "Unknown value \"" + attributeValue + "\" for attribute '" + attributeName + "'");
			}
			showExceptions = b ? OptionalBool::obTrue : OptionalBool::obFalse;
		}
		else if(attributeName == "showStacktrace") {
			bool b;
			if(stringToBool(b, attributeValue) == false) {
				throw XMLException(*this, "Unknown value \"" + attributeValue + "\" for attribute '" + attributeName + "'");
			}
			showStacktrace = b ? OptionalBool::obTrue : OptionalBool::obFalse;
		}
		else {
			throw XMLException(*this, "Unknown attribute '" + attributeName + "'");
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

void Exceptions::parseInnerElement(const tinyxml2::XMLElement& element) {
	if(element.Name() == nullptr) {
		throw XMLException(*this, "Element name is empty");
	}

	std::string elementName(element.Name());

	if(elementName == "document") {
		documents.push_back(ExceptionDocument(getFileName(), element));
	}
	else {
		throw XMLException(*this, "Unknown element name \"" + elementName + "\"");
	}
}

void Exceptions::save(std::ostream& oStream, std::size_t spaces) const {
	oStream << makeSpaces(spaces) << "<exceptions inheritDocuments=";

	if(inheritDocuments) {
		oStream << "\"true\"";
	}
	else {
		oStream << "\"false\"";
	}

	if(showExceptions == OptionalBool::obTrue) {
		oStream << " showExceptions=\"true\"";
	}
	else if(showExceptions == OptionalBool::obFalse) {
		oStream << " showExceptions=\"false\"";
	}

	if(showStacktrace == OptionalBool::obTrue) {
		oStream << " showStacktrace=\"true\"";
	}
	else if(showStacktrace == OptionalBool::obFalse) {
		oStream << " showStacktrace=\"false\"";
	}

	oStream << ">\n";

	for(const auto& entry : documents) {
		entry.save(oStream, spaces+2);
	}
	oStream << makeSpaces(spaces) << "<exceptions/>\n";
}

void Exceptions::install(engine::http::server::Context& engineHttpContext) const {
	/* ********************
	 * set showExceptions *
	 * ********************/
	if(showExceptions == OptionalBool::obTrue) {
		engineHttpContext.setShowException(engine::http::server::Context::obTrue);
	}
	else if(showExceptions == OptionalBool::obFalse) {
		engineHttpContext.setShowException(engine::http::server::Context::obFalse);
	}

	/* ********************
	 * set showStacktrace *
	 * ********************/
	if(showStacktrace == OptionalBool::obTrue) {
		engineHttpContext.setShowStacktrace(engine::http::server::Context::obTrue);
	}
	else if(showStacktrace == OptionalBool::obFalse) {
		engineHttpContext.setShowStacktrace(engine::http::server::Context::obFalse);
	}

	/***************************
	 * set exception documents *
	 * *************************/
	engineHttpContext.setInheritErrorDocuments(inheritDocuments);

	for(const auto& exceptionDocument : documents) {
		engineHttpContext.addErrorDocument(exceptionDocument.statusCode, exceptionDocument.path, exceptionDocument.parser);
	}
}

} /* namespace http */
} /* namespace config */
} /* namespace jerry */
