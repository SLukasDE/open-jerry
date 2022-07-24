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

#include <jerry/config/http/Context.h>
#include <jerry/config/http/EntryImpl.h>
#include <jerry/config/FilePosition.h>

#include <esl/object/Object.h>
#include <esl/utility/String.h>

namespace jerry {
namespace config {
namespace http {

Context::Context(const std::string& fileName, const tinyxml2::XMLElement& element)
: Config(fileName, element)
{
	if(element.GetUserData() != nullptr) {
		throw FilePosition::add(*this, "Element has user data but it should be empty");
	}

	bool hasInherit = false;

	for(const tinyxml2::XMLAttribute* attribute = element.FirstAttribute(); attribute != nullptr; attribute = attribute->Next()) {
		if(std::string(attribute->Name()) == "id") {
			if(id != "") {
				throw FilePosition::add(*this, "Multiple definition of attribute 'id'");
			}
			id = attribute->Value();
			if(id == "") {
				throw FilePosition::add(*this, "Invalid value \"\" for attribute 'id'");
			}
			if(refId != "") {
				throw FilePosition::add(*this, "Attribute 'id' is not allowed together with attribute 'ref-id'.");
			}
		}
		else if(std::string(attribute->Name()) == "ref-id") {
			if(refId != "") {
				throw FilePosition::add(*this, "Multiple definition of attribute 'ref-id'");
			}
			refId = attribute->Value();
			if(refId == "") {
				throw FilePosition::add(*this, "Invalid value \"\" for attribute 'ref-id'");
			}
			if(id != "") {
				throw FilePosition::add(*this, "Attribute 'ref-id' is not allowed together with attribute 'id'.");
			}
			if(hasInherit) {
				throw FilePosition::add(*this, "Attribute 'ref-id' is not allowed together with attribute 'inherit'.");
			}
		}
		else if(std::string(attribute->Name()) == "inherit") {
			std::string inheritStr = esl::utility::String::toLower(attribute->Value());
			if(hasInherit) {
				throw FilePosition::add(*this, "Multiple definition of attribute 'inherit'");
			}
			hasInherit = true;
			if(inheritStr == "true") {
				inherit = true;
			}
			else if(inheritStr == "false") {
				inherit = false;
			}
			else {
				throw FilePosition::add(*this, "Invalid value \"" + std::string(attribute->Value()) + "\" for attribute 'inherit'");
			}
			if(refId != "") {
				throw FilePosition::add(*this, "Attribute 'inherit' is not allowed together with attribute 'ref-id'.");
			}
		}
		else {
			throw FilePosition::add(*this, "Unknown attribute '" + std::string(attribute->Name()) + "'");
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

void Context::save(std::ostream& oStream, std::size_t spaces) const {
	oStream << makeSpaces(spaces) << "<context";

	if(refId.empty()) {
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
			entry->save(oStream, spaces+2);
		}

		for(const auto& responseHeader : responseHeaders) {
			responseHeader.saveResponseHeader(oStream, spaces+2);
		}

		exceptions.save(oStream, spaces+2);

		oStream << makeSpaces(spaces) << "</context>\n";
	}
	else {
		oStream << " ref-id=\"" << refId << "\"/>\n";
	}
}

void Context::install(engine::http::Context& engineHttpContext) const {
	if(refId.empty()) {
		std::unique_ptr<engine::http::Context> httpContext(new engine::http::Context(engineHttpContext.getProcessRegistry()));
		engine::http::Context& httpContextRef = *httpContext;

		if(inherit) {
			httpContextRef.setParent(&engineHttpContext);
		}

		if(id.empty()) {
			engineHttpContext.addContext(std::move(httpContext));
		}
		else {
			engineHttpContext.addObject(id, std::unique_ptr<esl::object::Object>(httpContext.release()));
		}

		/* *****************
		 * install entries *
		 * *****************/
		installEntries(httpContextRef);
	}
	else {
		engineHttpContext.addContext(refId);
	}
}

const std::string& Context::getId() const noexcept {
	return id;
}

const std::string& Context::getRefId() const noexcept {
	return refId;
}

bool Context::getInherit() const noexcept {
	return inherit;
}

void Context::installEntries(engine::http::Context& newContext) const {
	/* *****************
	 * install entries *
	 * *****************/
	for(const auto& entry : entries) {
		entry->install(newContext);
	}

	/* **********************
	 * Set response headers *
	 * **********************/
	for(const auto& responseHeader : responseHeaders) {
		newContext.addHeader(responseHeader.key, responseHeader.value);
	}

	exceptions.install(newContext);
}

void Context::parseInnerElement(const tinyxml2::XMLElement& element) {
	if(!refId.empty()) {
		throw FilePosition::add(*this, "No content allowed if 'ref-id' is specified.");
	}

	std::string innerElementName(element.Name());

	if(innerElementName == "response-header") {
		responseHeaders.push_back(Setting(getFileName(), element, false));
	}
	else if(innerElementName == "exceptions") {
		exceptions = Exceptions(getFileName(), element);
	}
	else {
		entries.emplace_back(new EntryImpl(getFileName(), element));
	}
}

} /* namespace http */
} /* namespace config */
} /* namespace jerry */
