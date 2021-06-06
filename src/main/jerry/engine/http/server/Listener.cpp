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

#include <jerry/engine/http/server/Listener.h>
#include <jerry/engine/http/server/Writer.h>
#include <jerry/engine/http/server/ExceptionHandler.h>
#include <jerry/engine/Engine.h>
#include <jerry/Module.h>
#include <jerry/Logger.h>

#include <esl/com/http/server/Connection.h>
#include <esl/utility/String.h>

#include <stdexcept>

namespace jerry {
namespace engine {
namespace http {
namespace server {

namespace {
Logger logger("jerry::engine::http::server::Listener");
}

Listener::EndpointEntry::EndpointEntry(const Endpoint* aEndpoint)
: endpoint(aEndpoint)
{ }

Listener::Listener(esl::object::ObjectContext& aEngineContext, bool inheritObjects, const std::string& aHostname, std::vector<std::string> aRefIds)
: Endpoint(*this, inheritObjects),
  engineContext(aEngineContext),
  hostname(aHostname),
  refIds(std::move(aRefIds)),
  rootEndpointEntry(this)
{ }

esl::object::Interface::Object* Listener::findHiddenObject(const std::string& id) const {
	esl::object::Interface::Object* object = Context::findLocalObject(id);

	if(object) {
		return object;
	}

	return engineContext.findObject<esl::object::Interface::Object>(id);
}

void Listener::dumpTree(std::size_t depth) const {
	for(std::size_t i=0; i<depth; ++i) {
		logger.info << "|   ";
	}
	logger.info << "Hostname: \"" + getHostname() + "\"\n";

	for(const auto& refId : refIds) {
		for(std::size_t i=0; i<depth; ++i) {
			logger.info << "|   ";
		}
		logger.info << "Added to: \"" + refId + "\"\n";
	}

	Endpoint::dumpTree(depth);
}

const std::vector<std::string>& Listener::getRefIds() const {
	return refIds;
}

const std::string& Listener::getHostname() const {
	return hostname;
}

esl::io::Input Listener::createRequestHandler(esl::com::http::server::RequestContext& baseRequestContext) {
	std::unique_ptr<Writer> writer(new Writer(*this, baseRequestContext));

	/* *************************************************************************** *
	 * Delegate to Context::createRequestHandler(Writer& writer) -> esl::io::Input *
	 * Returns a valid input object if a request handler has been found.           *
	 * *************************************************************************** */
	esl::io::Input input = Context::createRequestHandler(writer);

	if(!input) {
		/* *********************************************************************** *
		 * Throw a StatusCode(404) exception if no request handers has been found. *
		 * *********************************************************************** */

		ExceptionHandler exceptionHandler;

		exceptionHandler.setShowException(true);
		exceptionHandler.setShowStacktrace(false);

		//exceptionHandler.setMessage(esl::http::server::exception::StatusCode(404));
		exceptionHandler.call([]() {
			throw esl::com::http::server::exception::StatusCode(404);
		});

		/* send exception message on HTTP connection */
		exceptionHandler.dump(baseRequestContext.getConnection());
	}

	return input;
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

} /* namespace server */
} /* namespace http */
} /* namespace engine */
} /* namespace jerry */
