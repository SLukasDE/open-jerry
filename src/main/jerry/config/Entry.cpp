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

#include <jerry/config/Entry.h>
#include <jerry/config/RequestHandler.h>
#include <jerry/config/Context.h>
#include <jerry/config/Endpoint.h>

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

Entry::Entry(Entry&& other)
: requestHandler(other.requestHandler),
  context(other.context),
  endpoint(other.endpoint)
{
	other.requestHandler = nullptr;
	other.context = nullptr;
	other.endpoint = nullptr;
}

Entry::~Entry() {
	doDelete();
}

Entry& Entry::operator=(Entry&& other) {
	if(&other != this) {
		doDelete();

		requestHandler = other.requestHandler;
		context = other.context;
		endpoint = other.endpoint;

		other.requestHandler = nullptr;
		other.context = nullptr;
		other.endpoint = nullptr;
	}
	return *this;
}

void Entry::save(std::ostream& oStream, std::size_t spaces) const {
	if(getRequestHandler()) {
		// TODO
		getRequestHandler()->save(oStream, spaces);
	}
	else if(getContext()) {
		getContext()->save(oStream, spaces);
		//printContext(spaces, *getContext());
	}
	else if(getEndpoint()) {
		getEndpoint()->save(oStream, spaces);
		//printEndpoint(spaces, *entry.getEndpoint());
	}
}

void Entry::setRequestHandler(std::unique_ptr<RequestHandler> aRequestHandler) {
	if(requestHandler != aRequestHandler.get()) {
		doDelete();
		requestHandler = aRequestHandler.release();
	}
}

RequestHandler* Entry::getRequestHandler() const {
	return requestHandler;
}

void Entry::setContext(std::unique_ptr<Context> aContext) {
	if(context != aContext.get()) {
		doDelete();
		context = aContext.release();
	}
}

Context* Entry::getContext() const {
	return context;
}

void Entry::setEndpoint(std::unique_ptr<Endpoint> aEndpoint) {
	if(endpoint != aEndpoint.get()) {
		doDelete();
		endpoint = aEndpoint.release();
	}
}

Endpoint* Entry::getEndpoint() const {
	return endpoint;
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

	if(endpoint) {
		delete endpoint;
		endpoint = nullptr;
	}

}
} /* namespace config */
} /* namespace jerry */
