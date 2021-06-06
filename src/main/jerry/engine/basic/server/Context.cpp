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

#include <jerry/engine/basic/server/Context.h>
#include <jerry/engine/basic/server/Listener.h>
#include <jerry/Module.h>
#include <jerry/Logger.h>

#include <esl/com/basic/server/requesthandler/Interface.h>
#include <esl/object/InitializeContext.h>

#include <stdexcept>

namespace jerry {
namespace engine {
namespace basic {
namespace server {

namespace {
Logger logger("jerry::engine::messaging::server::Context");
} /* anonymous namespace */

Context::Context(Listener& aListener, const Context* aParentContext, bool aInheritObjects)
: listener(aListener),
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

Context& Context::addContext(bool inheritObjects) {
	Context* contextPtr = new Context(listener, this, inheritObjects);

	entries.push_back(Entry(std::unique_ptr<Context>(contextPtr)));

	return *contextPtr;
}

void Context::addRequestHandler(const std::string& implementation) {
	auto& requestHandler = jerry::getModule().getInterface<esl::com::basic::server::requesthandler::Interface>(implementation);

	entries.push_back(Entry(requestHandler));
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

		if(entry.requesthandler) {
			/* ********************* *
			 * handle RequestHandler *
			 * ********************* */
			for(std::size_t i=0; i<depth; ++i) {
				logger.info << "|   ";
			}
			logger.info << "+-> BasicHandler\n";
		}
	}
}

std::set<std::string> Context::getNotifier() const {
	std::set<std::string> notifier;

	for(auto& entry : entries) {
		if(entry.requesthandler) {
			std::set<std::string> tmpNotifier = entry.requesthandler->getNotifiers(*this);
			notifier.insert(tmpNotifier.begin(), tmpNotifier.end());
		}
	}

	return notifier;
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

esl::io::Input Context::createRequestHandler(std::unique_ptr<server::Writer>& writer) const {
	esl::io::Input input;

	for(auto& entry : entries) {
		if(entry.context) {
			/* ************** *
			 * handle Context *
			 * ************** */
			writer->getRequestContext().setContext(*entry.context);

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

		if(entry.requesthandler) {
			/* **************************** *
			 * handle RequestHandlerFactory *
			 * **************************** */
			writer->getRequestContext().setContext(*this);

			/* set requestHandlerFactory and call it */
			//requestHandler.setRequestHandler(entry.createRequestHandler);

			/* check if calling requestHandlerFactory got a valid requestHandler */
			/*
			if(requestHandler.hasRequestHandler()) {
				return true;
			}
			*/

			input = writer->getRequestContext().createInput(writer, entry.requesthandler->createInput);
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
} /* namespace basic */
} /* namespace engine */
} /* namespace jerry */
