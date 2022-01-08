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

#include <jerry/config/basic/EntryImpl.h>
#include <jerry/config/XMLException.h>

namespace jerry {
namespace config {
namespace basic {

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
	else if(elementName == "context" || elementName == "basic-context") {
		context = std::unique_ptr<Context>(new Context(getFileName(), element));
	}
	else if(elementName == "requesthandler") {
		requestHandler = std::unique_ptr<RequestHandler>(new RequestHandler(getFileName(), element));
	}
	else if(elementName == "client" || elementName == "basic-client") {
		basicClient = std::unique_ptr<Client>(new Client(getFileName(), element));
	}
	else if(elementName == "http-client") {
		httpClient = std::unique_ptr<http::Client>(new http::Client(getFileName(), element));
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
	if(context) {
		context->save(oStream, spaces);
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

void EntryImpl::install(engine::basic::Context& engineBasicContext) const {
	if(object) {
		object->install(engineBasicContext);
	}
	if(reference) {
		reference->install(engineBasicContext);
	}
	if(procedure) {
		procedure->install(engineBasicContext);
	}
	if(context) {
		context->install(engineBasicContext);
	}
	if(requestHandler) {
		requestHandler->install(engineBasicContext);
	}
	if(basicClient) {
		basicClient->install(engineBasicContext);
	}
	if(httpClient) {
		httpClient->install(engineBasicContext);
	}
}

} /* namespace basic */
} /* namespace config */
} /* namespace jerry */
