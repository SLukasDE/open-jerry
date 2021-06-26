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

#include <jerry/engine/http/server/Context.h>
#include <jerry/engine/http/server/Endpoint.h>
#include <jerry/engine/http/server/Listener.h>
#include <jerry/engine/http/server/Writer.h>
#include <jerry/engine/http/server/RequestContext.h>
#include <jerry/Module.h>
#include <jerry/Logger.h>

#include <esl/com/http/server/requesthandler/Interface.h>
#include <esl/object/InitializeContext.h>
#include <esl/utility/String.h>

#include <stdexcept>

namespace jerry {
namespace engine {
namespace http {
namespace server {

namespace {
Logger logger("jerry::engine::http::server::Context");

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

Context::Context(Listener& aListener, const Endpoint& aEndpoint, const Context* aParentContext, bool aInheritObjects)
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

esl::object::Interface::Object& Context::addObject(const std::string& id, const std::string& implementation, const esl::object::Interface::Settings& settings) {
	esl::object::Interface::Object* objectPtr = findLocalObject(id);
	if(objectPtr != nullptr) {
        throw std::runtime_error("Cannot add object with id '" + id + "', because there exists already an object with same id.");
	}

	esl::object::Interface::Object& object = BaseContext::addObject(id, implementation, settings);

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

Endpoint& Context::addEndpoint(std::string path, bool inheritObjects) {
	std::vector<std::string> pathList(esl::utility::String::split(esl::utility::String::trim(std::move(path), '/'), '/'));

	Endpoint* endpointPtr = new Endpoint(listener, endpoint, *this, std::move(pathList), inheritObjects);
	entries.push_back(Entry(std::unique_ptr<Endpoint>(endpointPtr)));

	return *endpointPtr;
}

void Context::addRequestHandler(const std::string& implementation) {
	entries.push_back(Entry(jerry::getModule().getInterface<esl::com::http::server::requesthandler::Interface>(implementation).createInput));
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

		if(entry.createRequestHandler) {
			/* ********************* *
			 * handle RequestHandler *
			 * ********************* */
			for(std::size_t i=0; i<depth; ++i) {
				logger.info << "|   ";
			}
			logger.info << "+-> RequestHandler\n";
		}
	}
}

esl::io::Input Context::createRequestHandler(std::unique_ptr<Writer>& writer) const {
	esl::io::Input input;

	for(auto& entry : entries) {
		if(entry.context) {
			/* ************** *
			 * handle Context *
			 * ************** */
			writer->getRequestContext().setContext(*entry.context);
			writer->getRequestContext().setEndpoint(getEndpoint());

			input = entry.context->createRequestHandler(writer);
			if(input) {
				return input;
			}
			/*
			if(entry.context->tryRequestHandler(writer)) {
				return true;
			}
			*/
		}

		if(entry.endpoint) {
			/* *************** *
			 * handle Endpoint *
			 * *************** */
			Endpoint* subEndpoint = entry.endpoint.get();

			if(isPathMatching(writer->getRequestContext().getPathList(), subEndpoint->getPathList(), subEndpoint->getDepth())) {
				Context* context = subEndpoint;

				writer->getRequestContext().setContext(*subEndpoint);
				writer->getRequestContext().setEndpoint(*subEndpoint);

				input = context->createRequestHandler(writer);
				if(input) {
					return input;
				}
				/*
				if(context->tryRequestHandler(writer)) {
					return true;
				}
				*/
			}
		}

		if(entry.createRequestHandler) {
			/* **************************** *
			 * handle RequestHandlerFactory *
			 * **************************** */
			writer->getRequestContext().setContext(*this);
			writer->getRequestContext().setEndpoint(getEndpoint());

			/* set requestHandlerFactory and call it */
			//requestHandler.setRequestHandler(entry.createRequestHandler);

			/* check if calling requestHandlerFactory got a valid requestHandler */
			/*
			if(requestHandler.hasRequestHandler()) {
				return true;
			}
			*/

			input = writer->getRequestContext().createRequestHandler(writer, entry.createRequestHandler);
			if(input) {
				return input;
			}
			//if(writer->getRequestContext().tryRequestHandler(entry.createRequestHandler)) {
			//	return esl::io::Input(std::unique_ptr<esl::io::Writer>(writer.release()));
			//}
		}
	}

	return input;
}

} /* namespace server */
} /* namespace http */
} /* namespace engine */
} /* namespace jerry */
