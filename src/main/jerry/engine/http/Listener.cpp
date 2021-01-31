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

#include <jerry/engine/http/Listener.h>
#include <jerry/engine/http/RequestHandler.h>
#include <jerry/engine/ExceptionHandler.h>
#include <jerry/engine/Engine.h>
#include <jerry/Module.h>
#include <jerry/Logger.h>

#include <esl/utility/String.h>

#include <stdexcept>

namespace jerry {
namespace engine {
namespace http {

namespace {
Logger logger("jerry::engine::http::Listener");
}

Listener::EndpointEntry::EndpointEntry(const Endpoint* aEndpoint)
: endpoint(aEndpoint)
{ }

Listener::Listener(Engine& aEngine, bool inheritObjects)
: Endpoint(*this, inheritObjects),
  engine(aEngine),
  rootEndpointEntry(this)
{ }

esl::object::Interface::Object* Listener::findHiddenObject(const std::string& id) const {
	esl::object::Interface::Object* object = Context::findLocalObject(id);

	if(object) {
		return object;
	}

	return engine.findObject(id);
}

std::unique_ptr<esl::http::server::requesthandler::Interface::RequestHandler> Listener::createRequestHandler(esl::http::server::RequestContext& baseRequestContext) {
	std::unique_ptr<RequestHandler> listenerRequestHandler(new RequestHandler(*this, baseRequestContext));

	if(Context::createRequestHandler(*listenerRequestHandler)) {
		/* convert unique_ptr<jerry::engine::RequestHandler> to unique_ptr<esl::...::RequestHandler> */
		std::unique_ptr<esl::http::server::requesthandler::Interface::RequestHandler> rv(listenerRequestHandler.release());
		return rv;
	}

	ExceptionHandler exceptionHandler;

	exceptionHandler.setShowException(true);
	exceptionHandler.setShowStacktrace(false);

//	exceptionHandler.setMessage(esl::http::server::exception::StatusCode(404));
	exceptionHandler.call([]() {
		throw esl::http::server::exception::StatusCode(404);
	});

	/* send exception message on HTTP connection */
	exceptionHandler.dump(baseRequestContext.getConnection());

	return std::unique_ptr<esl::http::server::requesthandler::Interface::RequestHandler>(new esl::http::server::requesthandler::Interface::RequestHandler);
}

void Listener::registerEndpoint(const Endpoint& endpoint) {
	EndpointEntry* endpointEntry = &rootEndpointEntry;

	std::vector<std::string> fullPathList = endpoint.getFullPathList();

	for(const auto& pathEntry : fullPathList) {
		EndpointEntry* tmpEndpointEntry = endpointEntry->entries[pathEntry].get();
		if(tmpEndpointEntry == nullptr) {
			tmpEndpointEntry = new EndpointEntry;
			//tmpEndpointEntry->depth = endpointEntry->depth + 1;
			endpointEntry->entries[pathEntry].reset(tmpEndpointEntry);
		}
		endpointEntry = tmpEndpointEntry;
	}

	if(endpointEntry->endpoint != nullptr) {
		std::string message = "ambiguous definition of endpoint \"";
		for(const auto& pathEntry : fullPathList) {
			message += "/" + pathEntry;
		}
		message += "\"";
		throw std::runtime_error(message);
	}

	endpointEntry->endpoint = &endpoint;
}

} /* namespace http */
} /* namespace engine */
} /* namespace jerry */
