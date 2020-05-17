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

#include <jerry/config/Context.h>
#include <jerry/config/Endpoint.h>
#include <jerry/config/RequestHandler.h>

#include <esl/Stacktrace.h>

#include <stdexcept>

namespace jerry {
namespace config {

namespace {
std::string makeSpaces(std::size_t spaces) {
	std::string rv;
	for(std::size_t i=0; i<spaces; ++i) {
		rv += " ";
	}
	return rv;
}
}

Context::Context(const tinyxml2::XMLElement& element) {
	if(element.GetUserData() != nullptr) {
		throw esl::addStacktrace(std::runtime_error("Element has user data but it should be empty (line " + std::to_string(element.GetLineNum()) + ")"));
	}

	for(const tinyxml2::XMLAttribute* attribute = element.FirstAttribute(); attribute != nullptr; attribute = attribute->Next()) {
		throw esl::addStacktrace(std::runtime_error(std::string("Unknown attribute \"") + attribute->Name() + "\" at line " + std::to_string(element.GetLineNum())));
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
			objects.push_back(Object(*innerElement));
		}
		else if(innerElementName == "reference") {
			references.push_back(Reference(*innerElement));
		}
		else if(innerElementName == "endpoint") {
			Entry entry;
			entry.setEndpoint(std::unique_ptr<Endpoint>(new Endpoint(*innerElement)));
			entries.push_back(std::move(entry));
		}
		else if(innerElementName == "context") {
			Entry entry;
			entry.setContext(std::unique_ptr<Context>(new Context(*innerElement)));
			entries.push_back(std::move(entry));
		}
		else if(innerElementName == "requesthandler") {
			Entry entry;
			entry.setRequestHandler(std::unique_ptr<RequestHandler>(new RequestHandler(*innerElement)));
			entries.push_back(std::move(entry));
		}
		else {
			throw esl::addStacktrace(std::runtime_error("Unknown element name \"" + std::string(innerElement->Name()) + "\" at line " + std::to_string(innerElement->GetLineNum())));
		}
	}
}

void Context::save(std::ostream& oStream, std::size_t spaces) const {
	oStream << makeSpaces(spaces) << "<context>\n";

	for(const auto& entry : objects) {
		entry.save(oStream, spaces+2);
	}

	for(const auto& entry : references) {
		entry.save(oStream, spaces+2);
	}

	for(const auto& entry : entries) {
		entry.save(oStream, spaces+2);
	}

	oStream << makeSpaces(spaces) << "<context/>\n";
}

} /* namespace config */
} /* namespace jerry */
