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

#include <jerry/config/http/EntryImpl.h>
#include <jerry/config/XMLException.h>

namespace jerry {
namespace config {
namespace http {

EntryImpl::EntryImpl(const std::string& fileName, const tinyxml2::XMLElement& element)
: Entry(fileName, element)
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
	else if(elementName == "database") {
		database = std::unique_ptr<Database>(new Database(getFileName(), element));
	}
	else if(elementName == "context" || elementName == "http-context" || elementName == "listener") {
		context = std::unique_ptr<Context>(new Context(getFileName(), element));
	}
	else if(elementName == "endpoint") {
		endpoint = std::unique_ptr<Endpoint>(new Endpoint(getFileName(), element));
	}
	else if(elementName == "host") {
		host = std::unique_ptr<Host>(new Host(getFileName(), element));
	}
	else if(elementName == "requesthandler") {
		requestHandler = std::unique_ptr<RequestHandler>(new RequestHandler(getFileName(), element));
	}
	else if(elementName == "basic-client") {
		basicClient = std::unique_ptr<basic::Client>(new basic::Client(getFileName(), element));
	}
	else if(elementName == "http-client" || elementName == "client") {
		httpClient = std::unique_ptr<Client>(new Client(getFileName(), element));
	}
	else {
		throw XMLException(*this, "Unknown element name \"" + elementName + "\"");
	}
}

void EntryImpl::save(std::ostream& oStream, std::size_t spaces) const {
	if(object) {
		object->save(oStream, spaces);
	}
	if(reference) {
		reference->save(oStream, spaces);
	}
	if(procedure) {
		procedure->save(oStream, spaces);
	}
	if(database) {
		database->save(oStream, spaces);
	}
	if(context) {
		context->save(oStream, spaces);
	}
	if(endpoint) {
		endpoint->save(oStream, spaces);
	}
	if(host) {
		host->save(oStream, spaces);
	}
	if(requestHandler) {
		requestHandler->save(oStream, spaces);
	}
	if(basicClient) {
		basicClient->save(oStream, spaces);
	}
	if(httpClient) {
		httpClient->save(oStream, spaces);
	}
}

void EntryImpl::install(engine::http::Context& engineHttpContext) const {
	if(object) {
		object->install(engineHttpContext);
	}
	if(reference) {
		reference->install(engineHttpContext);
	}
	if(procedure) {
		procedure->install(engineHttpContext);
	}
	if(database) {
		database->install(engineHttpContext);
	}
	if(context) {
		context->install(engineHttpContext);
	}
	if(endpoint) {
		endpoint->install(engineHttpContext);
	}
	if(host) {
		host->install(engineHttpContext);
	}
	if(requestHandler) {
		requestHandler->install(engineHttpContext);
	}
	if(basicClient) {
		basicClient->install(engineHttpContext);
	}
	if(httpClient) {
		httpClient->install(engineHttpContext);
	}
}

} /* namespace http */
} /* namespace config */
} /* namespace jerry */
