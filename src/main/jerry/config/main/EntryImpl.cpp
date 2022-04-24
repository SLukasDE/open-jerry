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

#include <jerry/config/main/EntryImpl.h>
#include <jerry/config/XMLException.h>
#include <jerry/engine/main/Context.h>

namespace jerry {
namespace config {
namespace main {

EntryImpl::EntryImpl(const std::string& fileName, const tinyxml2::XMLElement& element, bool isJBoot)
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
	else if(elementName == "database") {
		database = std::unique_ptr<Database>(new Database(getFileName(), element));
	}

	else if(elementName == "procedure") {
		procedure = std::unique_ptr<Procedure>(new Procedure(getFileName(), element));
	}
	else if(elementName == "procedure-context" && isJBoot == false) {
		procedureContext = std::unique_ptr<ProcedureContext>(new ProcedureContext(getFileName(), element));
	}

	else if(elementName == "basic-client") {
		basicClient = std::unique_ptr<basic::Client>(new basic::Client(getFileName(), element));
	}
	else if(elementName == "basic-context" && isJBoot == false) {
		basicContext = std::unique_ptr<BasicContext>(new BasicContext(getFileName(), element));
	}
	else if(elementName == "basic-server" && isJBoot == false) {
		basicServer = std::unique_ptr<basic::Server>(new basic::Server(getFileName(), element));
	}

	else if(elementName == "http-client") {
		httpClient = std::unique_ptr<http::Client>(new http::Client(getFileName(), element));
	}
	else if(elementName == "http-context" && isJBoot == false) {
		httpContext = std::unique_ptr<HttpContext>(new HttpContext(getFileName(), element));
	}
	else if(elementName == "http-server" && isJBoot == false) {
		httpServer = std::unique_ptr<http::Server>(new http::Server(getFileName(), element));
	}

	else {
		throw XMLException(*this, "Unknown element name \"" + elementName + "\".");
	}
}

void EntryImpl::save(std::ostream& oStream, std::size_t spaces) const {
	if(object) {
		object->save(oStream, spaces);
	}
	if(reference) {
		reference->save(oStream, spaces);
	}
	if(database) {
		database->save(oStream, spaces);
	}

	if(procedure) {
		procedure->save(oStream, spaces);
	}
	if(procedureContext) {
		procedureContext->save(oStream, spaces);
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
}

void EntryImpl::install(engine::main::Context& context) const {
	if(object) {
		object->install(context);
	}
	if(reference) {
		reference->install(context);
	}
	if(database) {
		database->install(context);
	}

	if(procedure) {
		procedure->install(context);
	}
	if(procedureContext) {
		procedureContext->install(context);
	}

	if(basicClient) {
		basicClient->install(context);
	}
	if(basicContext) {
		basicContext->install(context);
	}
	if(basicServer) {
		basicServer->install(context);
	}

	if(httpClient) {
		httpClient->install(context);
	}
	if(httpContext) {
		httpContext->install(context);
	}
	if(httpServer) {
		httpServer->install(context);
	}
}

} /* namespace main */
} /* namespace config */
} /* namespace jerry */
