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

#include <jerry/engine/messaging/Context.h>
#include <jerry/engine/messaging/Endpoint.h>
#include <jerry/engine/messaging/Listener.h>
//#include <jerry/engine/messaging/RequestHandler.h>
//#include <jerry/engine/messaging/RequestContext.h>
#include <jerry/Module.h>
#include <jerry/Logger.h>

#include <esl/messaging/messagehandler/Interface.h>
#include <esl/object/InitializeContext.h>

#include <stdexcept>

namespace jerry {
namespace engine {
namespace messaging {

namespace {
Logger logger("jerry::engine::messaging::Context");
} /* anonymous namespace */

Context::Context(Listener& aListener, const Endpoint* aEndpoint, const Context* aParentContext, bool aInheritObjects)
: listener(aListener),
  endpoint(aEndpoint),
  parentContext(aParentContext),
  inheritObjects(aInheritObjects)
{ }

void Context::addReference(const std::string& id, const std::string& refId) {
	esl::object::Interface::Object* objectPtr = findLocalObject(id);
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
	esl::object::Interface::Object* objectPtr = findLocalObject(id);
	if(objectPtr != nullptr) {
        throw std::runtime_error("Cannot add object with id '" + id + "', because there exists already an object with same id.");
	}

	esl::object::Interface::Object& object = BaseContext::addObject(id, implementation);

	localObjectsById[id] = &object;

	return object;
}

esl::object::Interface::Object* Context::findLocalObject(const std::string& id) const {
	auto localObjectsByIdIter = localObjectsById.find(id);
	if(localObjectsByIdIter == std::end(localObjectsById)) {
		return nullptr;
	}

    return localObjectsByIdIter->second;
}

esl::object::Interface::Object* Context::findHiddenObject(const std::string& id) const {
	esl::object::Interface::Object* object = findLocalObject(id);
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

esl::object::Interface::Object* Context::findObject(const std::string& id) const {
	if(inheritObjects) {
		return findHiddenObject(id);
	}
	else {
		return findLocalObject(id);
	}
}

Context& Context::addContext(bool inheritObjects) {
	Context* contextPtr = new Context(listener, endpoint, this, inheritObjects);

	entries.push_back(Entry(std::unique_ptr<Context>(contextPtr)));

	return *contextPtr;
}

Endpoint& Context::addEndpoint(std::string queueName, bool inheritObjects) {
	if(endpoint) {
        throw std::runtime_error("Cannot add endpoint for queue '" + queueName + "', because there exists already an endpoint for this context for queue \"" + endpoint->getQueueName() + "\".");
	}

	Endpoint* endpointPtr = new Endpoint(listener, *this, queueName, inheritObjects);
	entries.push_back(Entry(std::unique_ptr<Endpoint>(endpointPtr)));

	return *endpointPtr;
}

void Context::addMessageHandler(const std::string& implementation) {
	if(endpoint == nullptr) {
        throw std::runtime_error("Cannot add message handler without defined endpoint in context.");
	}

	esl::messaging::messagehandler::Interface::CreateMessageHandler createMessageHandler;
	createMessageHandler = jerry::getModule().getInterface<esl::messaging::messagehandler::Interface>(implementation).createMessageHandler;

	entries.push_back(Entry(createMessageHandler));
	listener.registerHandlerData(endpoint->getQueueName(), createMessageHandler, *this);
}

const Endpoint* Context::getEndpoint() const {
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
		esl::object::InitializeContext* initializeContext = dynamic_cast<esl::InitializeContext*>(objectEntry.second);
		if(initializeContext) {
			initializeContext->initializeContext(*this);
		}
	}
#endif

	// call initializeContext() of sub-context's
	for(auto& entry : entries) {
		if(entry.context) {
			/* ************** *
			 * handle Context *
			 * ************** */
			entry.context->initializeContext();
		}

		if(entry.endpoint) {
			/* *************** *
			 * handle Endpoint *
			 * *************** */
			entry.endpoint->initializeContext();
		}
	}
}

void Context::dumpTree(std::size_t depth) const {
	for(auto objectEntry : localObjectsById) {
		for(std::size_t i=0; i<depth; ++i) {
			logger.info << "|   ";
		}
		if(BaseContext::findObject(objectEntry.first)) {
			logger.info << "+-> Id: \"" << objectEntry.first << "\" -> " << objectEntry.second << "\n";
		}
		else {
			logger.info << "+-> Id: \"" << objectEntry.first << "\" -> " << objectEntry.second << " (reference)\n";
		}
	}

	// call dumpTree() of sub-context's
	for(auto& entry : entries) {
		if(entry.context) {
			/* ************** *
			 * handle Context *
			 * ************** */
			for(std::size_t i=0; i<depth; ++i) {
				logger.info << "|   ";
			}
			logger.info << "+-> Context:\n";
			entry.context->dumpTree(depth + 1);
		}

		if(entry.endpoint) {
			/* *************** *
			 * handle Endpoint *
			 * *************** */
			for(std::size_t i=0; i<depth; ++i) {
				logger.info << "|   ";
			}
			logger.info << "+-> Endpoint:\n";
			entry.endpoint->dumpTree(depth + 1);
		}

		if(entry.createMessageHandler) {
			/* ********************* *
			 * handle RequestHandler *
			 * ********************* */
			for(std::size_t i=0; i<depth; ++i) {
				logger.info << "|   ";
			}
			logger.info << "+-> MessageHandler\n";
		}
	}
}
#if 0
bool Context::createMessageHandler(MessageHandler& messageHandler) const {
	for(auto& entry : entries) {
		if(entry.context) {
			/* ************** *
			 * handle Context *
			 * ************** */
			requestHandler.setContext(*entry.context);
			requestHandler.setEndpoint(getEndpoint());

			if(entry.context->createRequestHandler(requestHandler)) {
				return true;
			}
		}

		if(entry.endpoint) {
			/* *************** *
			 * handle Endpoint *
			 * *************** */
			Endpoint* subEndpoint = entry.endpoint.get();

			if(isPathMatching(requestHandler.getPathList(), subEndpoint->getPathList(), subEndpoint->getDepth())) {
				Context* context = subEndpoint;

				requestHandler.setContext(*subEndpoint);
				requestHandler.setEndpoint(*subEndpoint);

				if(context->createRequestHandler(requestHandler)) {
					return true;
				}
			}
		}

		if(entry.createRequestHandler) {
			/* **************************** *
			 * handle RequestHandlerFactory *
			 * **************************** */
			requestHandler.setContext(*this);
			requestHandler.setEndpoint(getEndpoint());

			/* set requestHandlerFactory and call it */
			requestHandler.setRequestHandler(entry.createRequestHandler);

			/* check if calling requestHandlerFactory got a valid requestHandler */
			if(requestHandler.hasRequestHandler()) {
				return true;
			}
		}
	}

	return false;
}
#endif

} /* namespace messaging */
} /* namespace engine */
} /* namespace jerry */
