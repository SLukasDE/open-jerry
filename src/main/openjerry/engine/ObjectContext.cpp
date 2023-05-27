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

#include <openjerry/engine/ObjectContext.h>
#include <openjerry/engine/http/Context.h>
#include <openjerry/engine/http/Endpoint.h>
#include <openjerry/engine/http/Host.h>
#include <openjerry/Logger.h>

#include <esl/com/basic/client/ConnectionFactory.h>
#include <esl/com/http/client/ConnectionFactory.h>
#include <esl/processing/Procedure.h>
#include <esl/object/InitializeContext.h>

#include <stdexcept>

namespace openjerry {
namespace engine {

namespace {
Logger logger("openjerry::engine::ObjectContext");
} /* anonymous namespace */

ObjectContext::ObjectContext(ProcessRegistry* aProcessRegistry)
: processRegistry(aProcessRegistry)
{ }

void ObjectContext::setParent(esl::object::Context* objectContext) {
	parent = objectContext;
}

std::set<std::string> ObjectContext::getObjectIds() const {
	std::set<std::string> rv;

	if(parent) {
		rv = parent->getObjectIds();
	}

	for(const auto& object : objectRefsById) {
		rv.insert(object.first);
	}

	return rv;
}

void ObjectContext::addReference(const std::string& id, esl::object::Object& object) {
	logger.trace << "Adding object reference with id=\"" << id << "\"\n";

	if(id.empty()) {
		throw std::runtime_error("Add an object with empty id is not allowed.");
	}
	if(objectRefsById.count(id) != 0) {
        throw std::runtime_error("Cannot add an object reference with id '" + id + "' because there exists already a object reference with same id.");
	}

	objectRefsById.insert(std::make_pair(id, std::ref(object)));
}

void ObjectContext::initializeContext() {
	for(auto& object : objects) {
		ObjectContext* objectContext = dynamic_cast<ObjectContext*>(object.second.get());

		if(objectContext) {
			objectContext->initializeContext();
		}
		else {
			esl::object::InitializeContext* initializeContext = dynamic_cast<esl::object::InitializeContext*>(object.second.get());

			if(initializeContext) {
				initializeContext->initializeContext(*this);
			}
		}
	}
}

void ObjectContext::dumpTree(std::size_t depth) const {
	for(const auto& entry : objectRefsById) {
		const esl::object::Object* objectPtr = &entry.second.get();
		bool isReference = objects.count(entry.first) == 0;
		std::string isReferenceStr = isReference ? " (reference)" : "";

		const http::Context* httpContextPtr = dynamic_cast<const http::Context*>(objectPtr);
		const http::Endpoint* httpEndpointPtr = dynamic_cast<const http::Endpoint*>(objectPtr);
		const http::Host* httpHostPtr = dynamic_cast<const http::Host*>(objectPtr);

		const esl::processing::Procedure* procedurePtr = dynamic_cast<const esl::processing::Procedure*>(objectPtr);
		const esl::com::basic::client::ConnectionFactory* basicConnectionFactory = dynamic_cast<const esl::com::basic::client::ConnectionFactory*>(objectPtr);
		const esl::com::http::client::ConnectionFactory* httpConnectionFactory = dynamic_cast<const esl::com::http::client::ConnectionFactory*>(objectPtr);

		const ObjectContext* objectContext = dynamic_cast<const ObjectContext*>(objectPtr);

		for(std::size_t i=0; i<depth; ++i) {
			logger.info << "|   ";
		}

		if(httpEndpointPtr) {
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
			logger.info << "+-> HTTP context: \"" << entry.first << "\" -> " << httpContextPtr << isReferenceStr << "\n";
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

const std::map<std::string, std::reference_wrapper<esl::object::Object>>& ObjectContext::getObjects() const {
	return objectRefsById;
}

void ObjectContext::setProcessRegistry(ProcessRegistry* aProcessRegistry) {
	processRegistry = aProcessRegistry;
}

ProcessRegistry* ObjectContext::getProcessRegistry() {
	return processRegistry;
}

const ProcessRegistry* ObjectContext::getProcessRegistry() const {
	return processRegistry;
}

esl::object::Object* ObjectContext::findRawObject(const std::string& id) {
	// check if ID exist in objectsById
	auto iter = objectRefsById.find(id);
	if(iter != std::end(objectRefsById)) {
		return &iter->second.get();
	}

	// if id NOT exist in objectsById, then find object in parent ObjectContext
	return parent ? parent->findObject<esl::object::Object>(id) : nullptr;
}

const esl::object::Object* ObjectContext::findRawObject(const std::string& id) const {
	// check if ID exist in objectsById
	auto iter = objectRefsById.find(id);
	if(iter != std::end(objectRefsById)) {
		return &iter->second.get();
	}

	// if id NOT exist in objectsById, then find object in parent ObjectContext
	return parent ? parent->findObject<esl::object::Object>(id) : nullptr;
}

void ObjectContext::addRawObject(const std::string& id, std::unique_ptr<esl::object::Object> object) {
	logger.trace << "Adding object with id=\"" << id << "\"\n";

	if(id.empty()) {
		throw std::runtime_error("Add an object with empty id is not allowed.");
	}
	if(!object) {
		throw std::runtime_error("Cannot add an empty object with id '" + id + "'.");
	}

	addReference(id, *object);
	objects[id] = std::move(object);
}

} /* namespace engine */
} /* namespace openjerry */
