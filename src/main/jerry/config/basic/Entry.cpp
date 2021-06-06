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
#include <jerry/config/basic/Context.h>
#include <jerry/config/basic/RequestHandler.h>

#include <esl/Stacktrace.h>

#include <stdexcept>

namespace jerry {
namespace config {
namespace basic {

namespace {
std::string makeSpaces(std::size_t spaces) {
	std::string rv;
	for(std::size_t i=0; i<spaces; ++i) {
		rv += " ";
	}
	return rv;
}
}

Entry::Entry(Entry&& other)
: type(other.type),
  object(std::move(other.object)),
  reference(std::move(other.reference)),
  context(other.context),
  requestHandler(other.requestHandler),
  hasQueue(other.hasQueue)
{
	other.type = etNone;
	other.context = nullptr;
	other.requestHandler = nullptr;
}

Entry::Entry(const tinyxml2::XMLElement& element) {
	if(element.Name() == nullptr) {
		throw esl::addStacktrace(std::runtime_error("Element name is empty at line " + std::to_string(element.GetLineNum())));
	}

	std::string elementName(element.Name());

	if(elementName == "object") {
		type = etObject;
		object = std::unique_ptr<Object>(new Object(element));
	}
	else if(elementName == "reference") {
		type = etReference;
		reference = std::unique_ptr<Reference>(new Reference(element));
	}
	else if(elementName == "context") {
		type = etContext;
		context = new Context(element, false);
	}
	else if(elementName == "requesthandler") {
		type = etRequestHandler;
		requestHandler = new RequestHandler(element);
	}
	else {
		throw esl::addStacktrace(std::runtime_error("Unknown element name \"" + elementName + "\" at line " + std::to_string(element.GetLineNum())));
	}
}

Entry::~Entry() {
	doDelete();
}

Entry& Entry::operator=(Entry&& other) {
	if(&other != this) {
		doDelete();

		object = std::move(other.object);
		reference = std::move(other.reference);
		context = other.context;
		requestHandler = other.requestHandler;
		hasQueue = other.hasQueue;

		other.type = etNone;
		other.context = nullptr;
		other.requestHandler = nullptr;
	}
	return *this;
}

void Entry::save(std::ostream& oStream, std::size_t spaces) const {
	switch(getType()) {
	case etObject:
		getObject().save(oStream, spaces);
		break;
	case etReference:
		getReference().save(oStream, spaces);
		break;
	case etContext:
		getContext().save(oStream, spaces);
		break;
	case etRequestHandler:
		getRequestHandler().save(oStream, spaces);
		break;
	default:
		throw esl::addStacktrace(std::runtime_error("Entry::save() called, but Entry is empty"));
	}
}

Entry::Type Entry::getType() const {
	return type;
}

Object& Entry::getObject() const {
	if(object == nullptr) {
		throw esl::addStacktrace(std::runtime_error("Entry::getObject() called for empty object"));
	}
	return *object;
}

Reference& Entry::getReference() const {
	if(reference == nullptr) {
		throw esl::addStacktrace(std::runtime_error("Entry::getReference() called for empty reference"));
	}
	return *reference;
}

Context& Entry::getContext() const {
	if(context == nullptr) {
		throw esl::addStacktrace(std::runtime_error("Entry::getContext() called for empty context"));
	}
	return *context;
}

RequestHandler& Entry::getRequestHandler() const {
	if(requestHandler == nullptr) {
		throw esl::addStacktrace(std::runtime_error("Entry::getRequestHandler() called for empty requestHandler"));
	}
	return *requestHandler;
}

void Entry::doDelete() {
	if(requestHandler) {
		delete requestHandler;
		requestHandler = nullptr;
	}

	if(context) {
		delete context;
		context = nullptr;
	}
}

} /* namespace basic */
} /* namespace config */
} /* namespace jerry */
