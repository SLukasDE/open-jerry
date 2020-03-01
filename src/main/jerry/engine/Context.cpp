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

#include <jerry/engine/Context.h>
#include <jerry/engine/Endpoint.h>
#include <jerry/http/RequestHandler.h>
#include <jerry/http/RequestContext.h>
#include <jerry/Module.h>
#include <jerry/Logger.h>
#include <esl/http/server/handler/Interface.h>
#include <stdexcept>

namespace jerry {
namespace engine {

namespace {
Logger logger("jerry::engine::Context");
} /* anonymous namespace */

Context::Context(Listener& aListener, Context* aParent)
: listener(aListener),
  parent(aParent)
{ }

Context& Context::addContext() {
	Context* newContext = new Context(listener, this);

	registerContext(std::unique_ptr<Context>(newContext));

	return *newContext;
}

Endpoint& Context::addEndpoint(std::string path) {
	logger.trace << "Adding endpoint for path=\"" + path + "\"\n";

	Endpoint* newEndpoint = new Endpoint(listener, this, std::move(path));

	registerContext(std::unique_ptr<Context>(newEndpoint));

	return *newEndpoint;
}

void Context::addRequestHandler(const std::string& implementation) {
	logger.trace << "Adding requesthandler for implementation=\"" + implementation + "\"\n";

	requestHandlerFactories.push_back(jerry::getModule().getInterface<esl::http::server::handler::Interface>(implementation).createRequestHandler);
}

void Context::addReference(const std::string& id, const std::string& refId) {
	logger.trace << "Adding reference with id=\"" + id + "\" and reference-id=\"" + refId + "\"\n";

	auto allObjectsByIdIter = allObjectsById.find(id);
	if(allObjectsByIdIter != std::end(allObjectsById)) {
        throw std::runtime_error("Cannot add object with id '" + id + "', because there exists already an object with same name.");
	}

	esl::object::parameter::Interface::Object* object = getObjectWithEngine(refId);

	if(object == nullptr) {
        throw std::runtime_error("Cannot add object with id '" + id + "', because it's reference '" + refId + "' does not exists.");
	}

	allObjectsById[id] = object;
}

esl::object::parameter::Interface::Object& Context::addObject(const std::string& id, const std::string& implementation) {
	logger.trace << "Adding object with id=\"" + id + "\" and implementation=\"" + implementation + "\"\n";

	auto allObjectsByIdIter = allObjectsById.find(id);
	if(allObjectsByIdIter != std::end(allObjectsById)) {
        throw std::runtime_error("Cannot add object with id '" + id + "', because there exists already an object with same name.");
	}

	esl::object::parameter::Interface::Object& object = BaseContext::addObject(id, implementation);

	allObjectsById[id] = &object;

	return object;
}

esl::object::parameter::Interface::Object* Context::getObject(const std::string& id) const {
	logger.trace << "Lookup object with id=\"" + id + "\"\n";

	auto allObjectsByIdIter = allObjectsById.find(id);
	if(allObjectsByIdIter != std::end(allObjectsById)) {
		logger.trace << "Object found in Context\n";
        return allObjectsByIdIter->second;
	}

	if(parent) {
		logger.trace << "Object not found in Context, lookup in parent context.\n";
		return parent->getObject(id);
	}

	logger.trace << "Object not found in Context\n";
	return nullptr;
}

esl::object::parameter::Interface::Object* Context::getObjectWithEngine(const std::string& id) const {
	logger.trace << "Lookup object with id=\"" + id + "\"\n";

	auto allObjectsByIdIter = allObjectsById.find(id);
	if(allObjectsByIdIter != std::end(allObjectsById)) {
		logger.trace << "Object found in Context\n";
        return allObjectsByIdIter->second;
	}

	if(parent) {
		logger.trace << "Object not found in Context, lookup in parent context.\n";
		return parent->getObjectWithEngine(id);
	}

	logger.trace << "Object not found in Context\n";
	return nullptr;
}

std::vector<std::string> Context::getEndpointPathList() const {
	if(!parent) {
		return std::vector<std::string>();
	}
	return parent->getEndpointPathList();
}

void Context::registerContext(std::unique_ptr<Context> context) {
	contextList.push_back(std::move(context));
}

std::unique_ptr<esl::http::server::RequestHandler> Context::createRequestHandler(esl::http::server::RequestContext& baseRequestContext, const std::string& path, const Endpoint& endpoint) const {
	std::unique_ptr<esl::http::server::RequestContext> requestContext(new http::RequestContext(baseRequestContext, path, *this));
	std::unique_ptr<esl::http::server::RequestHandler> requestHandler;

	for(auto& requestHandlerFactory : requestHandlerFactories) {
		logger.trace << "Own requestHandlerFactory found.\n";
		requestHandler = requestHandlerFactory(*requestContext);
		if(requestHandler) {
			logger.trace << "Own requestHandlerFactory instantiated.\n";
			requestHandler.reset(new http::RequestHandler(std::move(requestHandler), std::move(requestContext), endpoint));
			return requestHandler;
		}
	}

	logger.trace << "Own requestHandlerFactory NOT found. Interate sub context.\n";
	for(const auto& context : contextList) {
		requestHandler = context->createRequestHandler(baseRequestContext, path, endpoint);
		if(requestHandler) {
			logger.trace << "Sub-Context requestHandlerFactory instantiated.\n";
			return requestHandler;
		}
	}

	return nullptr;
}

} /* namespace engine */
} /* namespace jerry */
