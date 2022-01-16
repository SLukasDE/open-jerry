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

#include <jerry/config/application/Entry.h>
#include <jerry/config/XMLException.h>

namespace jerry {
namespace config {
namespace application {

Entry::Entry(const std::string& fileName, const tinyxml2::XMLElement& element)
: Config(fileName, element)
{
	if(element.Name() == nullptr) {
		throw jerry::config::XMLException(*this, "Element name is empty");
	}

	std::string elementName(element.Name());

	if(elementName == "object") {
		object = std::unique_ptr<jerry::config::Object>(new jerry::config::Object(getFileName(), element));
	}
	else if(elementName == "reference" || elementName == "alias") {
		reference = std::unique_ptr<jerry::config::Reference>(new jerry::config::Reference(getFileName(), element));
	}
	else if(elementName == "procedure") {
		procedure = std::unique_ptr<jerry::config::Procedure>(new jerry::config::Procedure(getFileName(), element));
	}
	else if(elementName == "basic-client") {
		basicClient = std::unique_ptr<jerry::config::basic::Client>(new jerry::config::basic::Client(getFileName(), element));
	}
	else if(elementName == "basic-context") {
		basicContext = std::unique_ptr<jerry::config::basic::BasicContext>(new jerry::config::basic::BasicContext(getFileName(), element));
	}
	else if(elementName == "http-client") {
		httpClient = std::unique_ptr<jerry::config::http::Client>(new jerry::config::http::Client(getFileName(), element));
	}
	else if(elementName == "http-context") {
		httpContext = std::unique_ptr<jerry::config::http::HttpContext>(new jerry::config::http::HttpContext(getFileName(), element));
	}
	else {
		throw jerry::config::XMLException(*this, "Unknown element name \"" + elementName + "\"");
	}
}

void Entry::save(std::ostream& oStream, std::size_t spaces) const {
	if(object) {
		object->save(oStream, spaces);
	}
	if(reference) {
		reference->save(oStream, spaces);
	}
	if(procedure) {
		procedure->save(oStream, spaces);
	}
	if(basicClient) {
		basicClient->save(oStream, spaces);
	}
	if(basicContext) {
		basicContext->save(oStream, spaces);
	}
	if(httpClient) {
		httpClient->save(oStream, spaces);
	}
	if(httpContext) {
		httpContext->save(oStream, spaces);
	}
}

void Entry::install(engine::ObjectContext& engineObjectContext) const {
	if(object) {
		object->install(engineObjectContext);
	}
	if(reference) {
		reference->install(engineObjectContext);
	}
	if(procedure) {
		procedure->install(engineObjectContext);
	}
	if(basicClient) {
		basicClient->install(engineObjectContext);
	}
	if(basicContext) {
		basicContext->install(engineObjectContext);
	}
	if(httpClient) {
		httpClient->install(engineObjectContext);
	}
	if(httpContext) {
		httpContext->install(engineObjectContext);
	}
}

} /* namespace application */
} /* namespace config */
} /* namespace jerry */
