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

#include <jerry/config/messaging/Broker.h>

#include <esl/Stacktrace.h>
#include <esl/utility/String.h>

#include <stdexcept>

namespace jerry {
namespace config {
namespace messaging {

namespace {
std::string makeSpaces(std::size_t spaces) {
	std::string rv;
	for(std::size_t i=0; i<spaces; ++i) {
		rv += " ";
	}
	return rv;
}
}

Broker::Broker(const tinyxml2::XMLElement& element) {
	bool hasBrokers = false;

	if(element.GetUserData() != nullptr) {
		throw esl::addStacktrace(std::runtime_error("Element has user data but it should be empty (line " + std::to_string(element.GetLineNum()) + ")"));
	}

	for(const tinyxml2::XMLAttribute* attribute = element.FirstAttribute(); attribute != nullptr; attribute = attribute->Next()) {
		// 	<http-server id="http-1" implementation="mhd4esl" port="8080" https="true">
		if(std::string(attribute->Name()) == "id") {
			id = attribute->Value();
		}
		else if(std::string(attribute->Name()) == "implementation") {
			implementation = attribute->Value();
		}
		else if(std::string(attribute->Name()) == "brokers") {
			brokers = attribute->Value();
			hasBrokers = true;
		}
		else if(std::string(attribute->Name()) == "threads") {
			threads = std::stoi(std::string(attribute->Value()));
		}
		else {
			throw esl::addStacktrace(std::runtime_error(std::string("Unknown attribute \"") + attribute->Name() + "\" at line " + std::to_string(element.GetLineNum())));
		}
	}

	if(hasBrokers == false) {
		throw esl::addStacktrace(std::runtime_error(std::string("Missing attribute \"brokers\" at line ") + std::to_string(element.GetLineNum())));
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
			settings.push_back(Setting(*innerElement));
		}
		else {
			throw esl::addStacktrace(std::runtime_error("Unknown element name \"" + std::string(innerElement->Name()) + "\" at line " + std::to_string(innerElement->GetLineNum())));
		}
	}
}

void Broker::save(std::ostream& oStream, std::size_t spaces) const {
	// <message-broker id="broker-1" implementation="rdkafka4esl" brokers="localhost:9092" threads="4">
	oStream << makeSpaces(spaces) << "<message-broker id=\"" << id << "\" implementation=\"" << implementation << "\" brokers=\"" << brokers << "\" threads=\"" << threads << "\">\n";

	for(const auto& entry : settings) {
		entry.saveParameter(oStream, spaces+2);
	}

	oStream << makeSpaces(spaces) << "<message-broker/>\n";
}

} /* namespace messaging */
} /* namespace config */
} /* namespace jerry */
