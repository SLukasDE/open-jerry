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

#include <jerry/config/basic/Entry.h>
#include <jerry/config/XMLException.h>

namespace jerry {
namespace config {
namespace basic {

Entry::Entry(const Entry& other)
: Config(other),
  object(other.object ? new Object(*other.object) : nullptr),
  context(other.context ? new Context(*other.context) : nullptr),
  requestHandler(other.requestHandler ? new RequestHandler(*other.requestHandler) : nullptr)
{ }

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
	else if(elementName == "context") {
		context = std::unique_ptr<Context>(new Context(getFileName(), element, false));
	}
	else if(elementName == "requesthandler") {
		requestHandler = std::unique_ptr<RequestHandler>(new RequestHandler(getFileName(), element));
	}
	else {
		throw XMLException(*this, "Unknown element name \"" + elementName + "\"");
	}
}

void Entry::save(std::ostream& oStream, std::size_t spaces) const {
	if(object) {
		object->save(oStream, spaces);
	}
	if(context) {
		context->save(oStream, spaces);
	}
	if(requestHandler) {
		requestHandler->save(oStream, spaces);
	}
}

void Entry::install(engine::basic::server::Context& engineBasicContext) const {
	if(object) {
		object->install(engineBasicContext);
	}
	if(context) {
		context->install(engineBasicContext);
	}
	if(requestHandler) {
		requestHandler->install(engineBasicContext);
	}
}

} /* namespace basic */
} /* namespace config */
} /* namespace jerry */
