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
#include <jerry/engine/Listener.h>
#include <jerry/engine/RequestHandler.h>
#include <jerry/engine/RequestContext.h>
#include <jerry/Module.h>
#include <jerry/Logger.h>

#include <esl/http/server/InitializeContext.h>
#include <esl/utility/String.h>

#include <stdexcept>

namespace jerry {
namespace engine {

namespace {
Logger logger("jerry::engine::Context");

bool isPathMatching(const std::vector<std::string>& requestPathList, const std::vector<std::string>& endpointPathList, std::size_t startIndex) {
	if(endpointPathList.size() > requestPathList.size() - startIndex) {
		return false;
	}

	for(std::size_t index = 0; index < endpointPathList.size(); ++index) {
		if(requestPathList[startIndex + index] != endpointPathList[index]) {
			return false;
		}
	}

	return true;
}

} /* anonymous namespace */

Context::Context(Listener& aListener, const Endpoint& aEndpoint, const Context& aParentContext)
: listener(aListener),
  endpoint(aEndpoint),
  parentContext(&aParentContext)
{ }

// only used by Listener
Context::Context(Listener& aListener)
: listener(aListener),
  endpoint(aListener),
  parentContext(nullptr)
{ }

void Context::addReference(const std::string& id, const std::string& refId) {
	auto allObjectsByIdIter = allObjectsById.find(id);
	if(allObjectsByIdIter != std::end(allObjectsById)) {
        throw std::runtime_error("Cannot add object with id '" + id + "', because there exists already an object with same name.");
	}

	esl::object::Interface::Object* object = getHiddenObject(refId);

	if(object == nullptr) {
        throw std::runtime_error("Cannot add object with id '" + id + "', because it's reference '" + refId + "' does not exists.");
	}

	allObjectsById[id] = object;
}

esl::object::Interface::Object& Context::addObject(const std::string& id, const std::string& implementation) {
	auto allObjectsByIdIter = allObjectsById.find(id);
	if(allObjectsByIdIter != std::end(allObjectsById)) {
        throw std::runtime_error("Cannot add object with id '" + id + "', because there exists already an object with same name.");
	}

	esl::object::Interface::Object& object = BaseContext::addObject(id, implementation);

	allObjectsById[id] = &object;

	return object;
}

esl::object::Interface::Object* Context::getObject(const std::string& id) const {
	esl::object::Interface::Object* object = getLocalObject(id);
	if(object) {
		return object;
	}

	logger.warn << "Lookup for undefined object \"" << id << "\" in context.\n";
	return nullptr;
}

esl::object::Interface::Object* Context::getHiddenObject(const std::string& id) const {
	esl::object::Interface::Object* object = getLocalObject(id);
	if(object) {
		return object;
	}

	/* check if this is NOT an instance of Listener
	 * check could also be "if(&listener != this) { ... }" */
	if(parentContext) {
		return parentContext->getHiddenObject(id);
	}

	return nullptr;
}

Context& Context::addContext() {
	Context* newContext = new Context(listener, endpoint, *this);

	contextCreateRequestHandlerList.push_back(std::make_tuple(std::unique_ptr<Context>(newContext), nullptr, nullptr));

	return *newContext;
}

Endpoint& Context::addEndpoint(std::string path) {
	std::vector<std::string> pathList(esl::utility::String::split(esl::utility::String::trim(std::move(path), '/'), '/'));

	Endpoint* newEndpoint = new Endpoint(listener, endpoint, *this, std::move(pathList));
	contextCreateRequestHandlerList.push_back(std::make_tuple(nullptr, std::unique_ptr<Endpoint>(newEndpoint), nullptr));

	return *newEndpoint;
}

void Context::addRequestHandler(const std::string& implementation) {
	contextCreateRequestHandlerList.push_back(std::make_tuple(nullptr, nullptr, jerry::getModule().getInterface<esl::http::server::requesthandler::Interface>(implementation).createRequestHandler));
}

const Endpoint& Context::getEndpoint() const {
	return endpoint;
}

void Context::initializeContext() {
	// initialize objects of this context
	for(auto& object : allObjectsById) {
		esl::http::server::InitializeContext* initializeContext = dynamic_cast<esl::http::server::InitializeContext*>(object.second);
		if(initializeContext) {
			initializeContext->initializeContext(*this);
		}
	}

	// call initializeContext() of sub-context's
	for(auto& createRequestHandler : contextCreateRequestHandlerList) {
		if(std::get<0>(createRequestHandler)) {
			/* ************** *
			 * handle Context *
			 * ************** */
			Context* subContext = std::get<0>(createRequestHandler).get();
			subContext->initializeContext();
		}

		if(std::get<1>(createRequestHandler)) {
			/* *************** *
			 * handle Endpoint *
			 * *************** */
			Endpoint* subEndpoint = std::get<1>(createRequestHandler).get();
			subEndpoint->initializeContext();
		}
	}
}

bool Context::createRequestHandler(RequestHandler& requestHandler) const {
	for(auto& createRequestHandler : contextCreateRequestHandlerList) {
		if(std::get<0>(createRequestHandler)) {
			/* ************** *
			 * handle Context *
			 * ************** */
			Context* subContext = std::get<0>(createRequestHandler).get();

			requestHandler.setContext(*subContext);
			requestHandler.setEndpoint(getEndpoint());

			if(std::get<0>(createRequestHandler)->createRequestHandler(requestHandler)) {
				return true;
			}
		}

		if(std::get<1>(createRequestHandler)) {
			/* *************** *
			 * handle Endpoint *
			 * *************** */
			Endpoint* subEndpoint = std::get<1>(createRequestHandler).get();

			if(isPathMatching(requestHandler.getPathList(), subEndpoint->getPathList(), subEndpoint->getDepth())) {
				Context* context = subEndpoint;

				requestHandler.setContext(*subEndpoint);
				requestHandler.setEndpoint(*subEndpoint);

				if(context->createRequestHandler(requestHandler)) {
					return true;
				}
			}
		}

		if(std::get<2>(createRequestHandler)) {
			/* **************************** *
			 * handle RequestHandlerFactory *
			 * **************************** */
			requestHandler.setContext(*this);
			requestHandler.setEndpoint(getEndpoint());
			requestHandler.setRequestHandler(std::get<2>(createRequestHandler));

			if(requestHandler.hasRequestHandler()) {
				return true;
			}
		}
	}

	return false;
}

esl::object::Interface::Object* Context::getLocalObject(const std::string& id) const {
	auto allObjectsByIdIter = allObjectsById.find(id);
	if(allObjectsByIdIter != std::end(allObjectsById)) {
        return allObjectsByIdIter->second;
	}

	return nullptr;
}

} /* namespace engine */
} /* namespace jerry */
