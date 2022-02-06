/*
 * This file is part of Jerry application server.
 * Copyright (C) 2020-2022 Sven Lukas
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

#include <jerry/engine/ObjectContext.h>
#include <jerry/engine/basic/Context.h>
#include <jerry/engine/http/Context.h>
#include <jerry/engine/http/Endpoint.h>
#include <jerry/engine/http/Host.h>
#include <jerry/Logger.h>

#include <esl/com/basic/client/Interface.h>
#include <esl/com/http/client/Interface.h>
#include <esl/processing/procedure/Interface.h>
#include <esl/object/InitializeContext.h>

#include <stdexcept>

namespace jerry {
namespace engine {

namespace {
Logger logger("jerry::engine::ObjectContext");
} /* anonymous namespace */

ObjectContext::ObjectContext(bool aIsGlobal)
: isGlobal(aIsGlobal)
{ }

void ObjectContext::setParent(esl::object::ObjectContext* objectContext) {
	parent = objectContext;
}

void ObjectContext::addObject(const std::string& id, std::unique_ptr<esl::object::Interface::Object> object) {
	logger.trace << "Adding object with id=\"" << id << "\"\n";

	if(!object) {
		throw std::runtime_error("Cannot add an empty object with id '" + id + "'.");
	}

	addReference(id, *object);
	objects[id] = std::move(object);
}

void ObjectContext::addReference(const std::string& id, esl::object::Interface::Object& object) {
	logger.trace << "Adding object reference with id=\"" << id << "\"\n";

	if(objectRefsById.count(id) != 0) {
        throw std::runtime_error("Cannot add an object reference with id '" + id + "' because there exists already a object reference with same id.");
	}

	objectRefsById.insert(std::make_pair(id, std::ref(object)));
}

void ObjectContext::initializeContext() {
	for(auto& object : objects) {
		esl::object::InitializeContext* initializeContext = dynamic_cast<esl::object::InitializeContext*>(object.second.get());
		ObjectContext* objectContext = dynamic_cast<ObjectContext*>(object.second.get());

		if(initializeContext) {
			initializeContext->initializeContext(*this);
		}

		if(objectContext) {
			objectContext->initializeContext();
		}
	}
}

void ObjectContext::dumpTree(std::size_t depth) const {
	for(const auto& entry : objectRefsById) {
		const esl::object::Interface::Object* objectPtr = &entry.second.get();
		bool isReference = objects.count(entry.first) == 0;
		std::string isReferenceStr = isReference ? " (reference)" : "";

		const basic::Context* basicContextPtr = dynamic_cast<const basic::Context*>(objectPtr);
		const http::Context* httpContextPtr = dynamic_cast<const http::Context*>(objectPtr);
		const http::Endpoint* httpEndpointPtr = dynamic_cast<const http::Endpoint*>(objectPtr);
		const http::Host* httpHostPtr = dynamic_cast<const http::Host*>(objectPtr);

		const esl::processing::procedure::Interface::Procedure* procedurePtr = dynamic_cast<const esl::processing::procedure::Interface::Procedure*>(objectPtr);
		const esl::com::basic::client::Interface::ConnectionFactory* basicConnectionFactory = dynamic_cast<const esl::com::basic::client::Interface::ConnectionFactory*>(objectPtr);
		const esl::com::http::client::Interface::ConnectionFactory* httpConnectionFactory = dynamic_cast<const esl::com::http::client::Interface::ConnectionFactory*>(objectPtr);

		const ObjectContext* objectContext = dynamic_cast<const ObjectContext*>(objectPtr);

		for(std::size_t i=0; i<depth; ++i) {
			logger.info << "|   ";
		}

		if(basicContextPtr) {
			if(isGlobal) {
				logger.info << "+-> Basic context: \"" << entry.first << "\" -> " << objectPtr << isReferenceStr << "\n";
			}
			else {
				logger.info << "+-> Context: \"" << entry.first << "\" -> " << objectPtr << isReferenceStr << "\n";
			}
			if(!isReference) {

				basicContextPtr->dumpTree(depth+1);
			}
		}
		else if(httpEndpointPtr) {
			logger.info << "+-> Endpoint \"" << entry.first << "\" -> " << httpEndpointPtr << isReferenceStr << "\n";
			if(!isReference) {
				httpEndpointPtr->dumpTree(depth+1);
			}
		}
		else if(httpHostPtr) {
			logger.info << "+-> Host \"" << entry.first << "\" -> " << httpHostPtr << isReferenceStr << "\n";
			if(!isReference) {
				httpHostPtr->dumpTree(depth+1);
			}
		}
		else if(httpContextPtr) {
			if(isGlobal) {
				logger.info << "+-> HTTP context: \"" << entry.first << "\" -> " << httpContextPtr << isReferenceStr << "\n";
			}
			else {
				logger.info << "+-> Context: \"" << entry.first << "\" -> " << httpContextPtr << isReferenceStr << "\n";
			}
			if(!isReference) {
				httpContextPtr->dumpTree(depth+1);
			}
		}
		else if(basicConnectionFactory) {
			logger.info << "+-> Basic-Client: \"" << entry.first << "\" -> " << basicConnectionFactory << isReferenceStr << "\n";
		}
		else if(httpConnectionFactory) {
			logger.info << "+-> HTTP-Client: \"" << entry.first << "\" -> " << httpConnectionFactory << isReferenceStr << "\n";
		}
		else if(procedurePtr) {
			logger.info << "+-> Procedure: \"" << entry.first << "\" -> " << procedurePtr << isReferenceStr << "\n";
		}
		else if(objectContext) {
			logger.info << "+-> Object context: \"" << entry.first << "\" -> " << objectPtr << isReferenceStr << "\n";
		}
		else {
			logger.info << "+-> Object: \"" << entry.first << "\" -> " << objectPtr << isReferenceStr << "\n";
		}
	}
}

const std::map<std::string, std::reference_wrapper<esl::object::Interface::Object>>& ObjectContext::getObjects() const {
	return objectRefsById;
}

esl::object::Interface::Object* ObjectContext::findRawObject(const std::string& id) {
	// check if ID exist in objectsById
	auto iter = objectRefsById.find(id);
	if(iter != std::end(objectRefsById)) {
		return &iter->second.get();
	}

	// if id NOT exist in objectsById, then find object in parent ObjectContext
	return parent ? parent->findObject<esl::object::Interface::Object>(id) : nullptr;
}

const esl::object::Interface::Object* ObjectContext::findRawObject(const std::string& id) const {
	// check if ID exist in objectsById
	auto iter = objectRefsById.find(id);
	if(iter != std::end(objectRefsById)) {
		return &iter->second.get();
	}

	// if id NOT exist in objectsById, then find object in parent ObjectContext
	return parent ? parent->findObject<esl::object::Interface::Object>(id) : nullptr;
}

} /* namespace engine */
} /* namespace jerry */
