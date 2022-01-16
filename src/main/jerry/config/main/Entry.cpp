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

#include <jerry/config/main/Entry.h>
#include <jerry/config/XMLException.h>

namespace jerry {
namespace config {
namespace main {

Entry::Entry(const std::string& fileName, const tinyxml2::XMLElement& element)
: Config(fileName, element)
{
	if(element.Name() == nullptr) {
		throw XMLException(*this, "Element name is empty");
	}

	std::string elementName(element.Name());

	if(elementName == "object") {
		object = std::unique_ptr<Object>(new Object(getFileName(), element));
	}
	else if(elementName == "reference" || elementName == "alias") {
		reference = std::unique_ptr<Reference>(new Reference(getFileName(), element));
	}
	else if(elementName == "procedure") {
		procedure = std::unique_ptr<Procedure>(new Procedure(getFileName(), element));
	}
	else if(elementName == "basic-client") {
		basicClient = std::unique_ptr<basic::Client>(new basic::Client(getFileName(), element));
	}
	else if(elementName == "basic-context") {
		basicContext = std::unique_ptr<basic::BasicContext>(new basic::BasicContext(getFileName(), element));
	}
	else if(elementName == "basic-server") {
		basicServer = std::unique_ptr<basic::Server>(new basic::Server(getFileName(), element));
	}
	else if(elementName == "http-client") {
		httpClient = std::unique_ptr<http::Client>(new http::Client(getFileName(), element));
	}
	else if(elementName == "http-context") {
		httpContext = std::unique_ptr<http::HttpContext>(new http::HttpContext(getFileName(), element));
	}
	else if(elementName == "http-server") {
		httpServer = std::unique_ptr<http::Server>(new http::Server(getFileName(), element));
	}
	else if(elementName == "daemon") {
		daemon = std::unique_ptr<daemon::Daemon>(new daemon::Daemon(getFileName(), element));
	}
	else {
		throw XMLException(*this, "Unknown element name \"" + elementName + "\"");
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
	if(basicServer) {
		basicServer->save(oStream, spaces);
	}
	if(httpClient) {
		httpClient->save(oStream, spaces);
	}
	if(httpContext) {
		httpContext->save(oStream, spaces);
	}
	if(httpServer) {
		httpServer->save(oStream, spaces);
	}
	if(daemon) {
		daemon->save(oStream, spaces);
	}
}

void Entry::install(engine::Engine& engine) const {
	if(object) {
		object->install(engine);
	}
	if(reference) {
		reference->install(engine);
	}
	if(procedure) {
		procedure->install(engine);
	}
	if(basicClient) {
		basicClient->install(engine);
	}
	if(basicContext) {
		basicContext->install(engine);
	}
	if(basicServer) {
		basicServer->install(engine);
	}
	if(httpClient) {
		httpClient->install(engine);
	}
	if(httpContext) {
		httpContext->install(engine);
	}
	if(httpServer) {
		httpServer->install(engine);
	}
	if(daemon) {
		daemon->install(engine);
	}
}

} /* namespace main */
} /* namespace config */
} /* namespace jerry */
