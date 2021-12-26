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

#include <jerry/config/basic/Context.h>
#include <jerry/config/basic/Entry.h>
#include <jerry/config/basic/RequestHandler.h>
#include <jerry/config/Object.h>
#include <jerry/config/XMLException.h>

#include <esl/Stacktrace.h>
#include <esl/utility/String.h>

#include <stdexcept>

namespace jerry {
namespace config {
namespace basic {

Context::Context(const std::string& fileName, const tinyxml2::XMLElement& element, Nature aNature)
: Config(fileName, element),
  nature(aNature)
{
	if(element.GetUserData() != nullptr) {
		throw XMLException(*this, "Element has user data but it should be empty");
	}

	bool hasInherit = false;

	for(const tinyxml2::XMLAttribute* attribute = element.FirstAttribute(); attribute != nullptr; attribute = attribute->Next()) {
		if(std::string(attribute->Name()) == "id") {
			id = attribute->Value();
			if(nature == listener) {
				throw XMLException(*this, "Attribute 'id' is not allowed.");
			}
			if(id == "") {
				throw XMLException(*this, "Invalid value \"\" for attribute 'id'");
			}
			if(refId != "") {
				throw XMLException(*this, "Attribute 'id' is not allowed together with attribute 'ref-id'.");
			}
		}
		else if(std::string(attribute->Name()) == "ref-id") {
			refId = attribute->Value();
			if(nature == listener) {
				throw XMLException(*this, "Attribute 'ref-id' is not allowed.");
			}
			if(nature == globalContext) {
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

	if(nature == globalContext && id == "") {
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
	switch(nature) {
	case globalContext:
		oStream << makeSpaces(spaces) << "<basic-context";
		break;
	case listener:
		oStream << makeSpaces(spaces) << "<listener";
		break;
	case context:
		oStream << makeSpaces(spaces) << "<context";
		break;
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

		switch(nature) {
		case globalContext:
			oStream << makeSpaces(spaces) << "</basic-context>\n";
			break;
		case listener:
			oStream << makeSpaces(spaces) << "</listener>\n";
			break;
		case context:
			oStream << makeSpaces(spaces) << "</context>\n";
			break;
		}
	}
}

void Context::install(engine::Engine& engine) const {
	std::unique_ptr<engine::basic::server::Context> context(new engine::basic::server::Context);

	if(inherit) {
		context->ObjectContext::setParent(&engine);
	}

	/* *****************
	 * install entries *
	 * *****************/
	for(const auto& entry : entries) {
		entry.install(*context);
	}

	engine.addObject(id, std::unique_ptr<esl::object::Interface::Object>(context.release()));
}

void Context::install(engine::basic::server::Context& engineBasicContext) const {
	if(refId == "") {
		engine::basic::server::Context& newEngineContext = engineBasicContext.addContext(id, inherit);

		/* *****************
		 * install entries *
		 * *****************/
		for(const auto& entry : entries) {
			entry.install(newEngineContext);
		}
	}
	else {
		engineBasicContext.addContext(refId);
	}
}

void Context::parseInnerElement(const tinyxml2::XMLElement& element) {
	entries.push_back(Entry(getFileName(), element));
}

} /* namespace basic */
} /* namespace config */
} /* namespace jerry */
