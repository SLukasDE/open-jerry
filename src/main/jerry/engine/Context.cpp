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
	esl::object::Interface::Object* objectPtr = findObject(id);
	if(objectPtr != nullptr) {
        throw std::runtime_error("Cannot add reference with id '" + id + "', because there exists already an object with same id.");
	}

	objectPtr = findHiddenObject(refId);
	if(objectPtr == nullptr) {
        throw std::runtime_error("Cannot add reference with id '" + id + "', because it's reference id '" + refId + "' does not exists.");
	}

	localObjectsById[id] = objectPtr;
}

esl::object::Interface::Object& Context::addObject(const std::string& id, const std::string& implementation) {
	esl::object::Interface::Object* objectPtr = findObject(id);
	if(objectPtr != nullptr) {
        throw std::runtime_error("Cannot add object with id '" + id + "', because there exists already an object with same id.");
	}

	esl::object::Interface::Object& object = BaseContext::addObject(id, implementation);

	localObjectsById[id] = &object;

	return object;
}

esl::object::Interface::Object* Context::findObject(const std::string& id) const {
	auto localObjectsByIdIter = localObjectsById.find(id);
	if(localObjectsByIdIter == std::end(localObjectsById)) {
		return nullptr;
	}

    return localObjectsByIdIter->second;
}

esl::object::Interface::Object* Context::findHiddenObject(const std::string& id) const {
	esl::object::Interface::Object* object = findObject(id);
	if(object) {
		return object;
	}

	/* check if this is NOT an instance of Listener
	 * check could also be "if(&listener != this) { ... }" */
	if(parentContext) {
		return parentContext->findHiddenObject(id);
	}

	return nullptr;
}

Context& Context::addContext() {
	Context* contextPtr = new Context(listener, endpoint, *this);

	contextCreateRequestHandlerList.push_back(std::make_tuple(std::unique_ptr<Context>(contextPtr), nullptr, nullptr));

	return *contextPtr;
}

Endpoint& Context::addEndpoint(std::string path) {
	std::vector<std::string> pathList(esl::utility::String::split(esl::utility::String::trim(std::move(path), '/'), '/'));

	Endpoint* endpointPtr = new Endpoint(listener, endpoint, *this, std::move(pathList));
	contextCreateRequestHandlerList.push_back(std::make_tuple(nullptr, std::unique_ptr<Endpoint>(endpointPtr), nullptr));

	return *endpointPtr;
}

void Context::addRequestHandler(const std::string& implementation) {
	contextCreateRequestHandlerList.push_back(std::make_tuple(nullptr, nullptr, jerry::getModule().getInterface<esl::http::server::requesthandler::Interface>(implementation).createRequestHandler));
}

const Endpoint& Context::getEndpoint() const {
	return endpoint;
}

void Context::initializeContext() {
	// initialize objects of this context

#if 1
	// Initialize owned objects only, not references
	BaseContext::initializeContext();
#else
	// Don't initialize references
	for(auto& objectEntry : localObjectsById) {
		esl::http::server::InitializeContext* initializeContext = dynamic_cast<esl::http::server::InitializeContext*>(objectEntry.second);
		if(initializeContext) {
			initializeContext->initializeContext(*this);
		}
	}
#endif

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

void Context::dumpTree(std::size_t depth) const {
	for(auto objectEntry : localObjectsById) {
		for(std::size_t i=0; i<depth; ++i) {
			logger.info << "|  ";
		}
		if(BaseContext::findObject(objectEntry.first)) {
			logger.info << "+-> Id: \"" << objectEntry.first << "\" -> " << objectEntry.second << "\n";
		}
		else {
			logger.info << "+-> Id: \"" << objectEntry.first << "\" -> " << objectEntry.second << " (reference)\n";
		}
	}

	// call dumpTree() of sub-context's
	for(auto& createRequestHandler : contextCreateRequestHandlerList) {
		if(std::get<0>(createRequestHandler)) {
			/* ************** *
			 * handle Context *
			 * ************** */
			Context* subContext = std::get<0>(createRequestHandler).get();
			subContext->dumpTree(depth + 1);
		}

		if(std::get<1>(createRequestHandler)) {
			/* *************** *
			 * handle Endpoint *
			 * *************** */
			Endpoint* subEndpoint = std::get<1>(createRequestHandler).get();
			subEndpoint->dumpTree(depth + 1);
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

			/* set requestHandlerFactory and call it */
			requestHandler.setRequestHandler(std::get<2>(createRequestHandler));

			/* check if calling requestHandlerFactory got a valid requestHandler */
			if(requestHandler.hasRequestHandler()) {
				return true;
			}
		}
	}

	return false;
}

} /* namespace engine */
} /* namespace jerry */
