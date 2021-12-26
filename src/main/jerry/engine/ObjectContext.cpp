/*
MIT License
Copyright (c) 2019-2021 Sven Lukas

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:
The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
*/

#include <jerry/engine/ObjectContext.h>
#include <jerry/engine/basic/server/Context.h>
#include <jerry/engine/http/server/Context.h>
#include <jerry/Logger.h>

#include <esl/com/basic/client/Interface.h>
#include <esl/com/http/client/Interface.h>
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

void ObjectContext::setParent(esl::object::Interface::ObjectContext* objectContext) {
	parent = objectContext;
}

esl::object::Interface::Object& ObjectContext::addObject(const std::string& id, const std::string& implementation, const esl::object::Interface::Settings& settings) {
	logger.trace << "Adding object with id=\"" << id << "\" and implementation=\"" << implementation << "\"\n";

	std::unique_ptr<esl::object::Interface::Object> object = esl::getModule().getInterface<esl::object::Interface>(implementation).createObject(settings);
	if(!object) {
		throw std::runtime_error("Cannot create an object with id '" + id + "' for implementation '" + implementation + "' because interface method createObject() returns nullptr.");
	}

	esl::object::Interface::Object* objectPtr = object.get();
	addObject(id, std::move(object));

	return *objectPtr;
}

void ObjectContext::addObject(const std::string& id, std::unique_ptr<esl::object::Interface::Object> object) {
	logger.trace << "Adding object with id=\"" << id << "\"\n";

	if(!object) {
		throw std::runtime_error("Cannot add an empty object with id '" + id + "'.");
	}

	addReference(id, *object);
	objects[id] = std::move(object);
}

void ObjectContext::addReference(const std::string& id, const std::string& refId) {
	esl::object::Interface::Object* objectPtr = findRawObject(refId);
	if(objectPtr == nullptr) {
        throw std::runtime_error("Cannot add reference with id '" + id + "', because it's reference id '" + refId + "' does not exists.");
	}

	addReference(id, *objectPtr);
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
		std::string isReferenceStr = objects.count(entry.first) == 0 ? " (reference)" : "";

		const basic::server::Context* basicContextPtr = dynamic_cast<const basic::server::Context*>(objectPtr);
		const http::server::Context* httpContextPtr = dynamic_cast<const http::server::Context*>(objectPtr);

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
			basicContextPtr->dumpTree(depth+1);
		}

		else if(httpContextPtr) {
			if(isGlobal) {
				logger.info << "+-> HTTP context: \"" << entry.first << "\" -> " << httpContextPtr << "\n";
			}
			else {
				logger.info << "+-> Context: \"" << entry.first << "\" -> " << httpContextPtr << "\n";
			}
			httpContextPtr->dumpTree(depth+1);
		}

		else if(basicConnectionFactory) {
			logger.info << "+-> Basic-Client: \"" << entry.first << "\" -> " << basicConnectionFactory << "\n";
		}

		else if(httpConnectionFactory) {
			logger.info << "+-> HTTP-Client: \"" << entry.first << "\" -> " << httpConnectionFactory << "\n";
		}

		else if(objectContext) {
			logger.info << "+-> Object context: \"" << entry.first << "\" -> " << objectPtr << isReferenceStr << "\n";
		}
		else {
			logger.info << "+-> Object: \"" << entry.first << "\" -> " << objectPtr << isReferenceStr << "\n";
		}
	}
}

esl::object::Interface::Object* ObjectContext::getObject() const {
	auto iter = objectRefsById.find("");
	return iter == std::end(objectRefsById) ? nullptr : &iter->second.get();
}

const std::map<std::string, std::reference_wrapper<esl::object::Interface::Object>>& ObjectContext::getObjects() const {
	return objectRefsById;
}

esl::object::Interface::Object* ObjectContext::findRawObject(const std::string& id) const {
	// check if ID exist in objectsById
	auto iter = objectRefsById.find(id);
	if(iter != std::end(objectRefsById)) {
		return &iter->second.get();
		//const esl::object::ObjectContext* objectContext = &iter->second.get();
		//return objectContext ? objectContext->findObject<esl::object::Interface::Object>(id) : nullptr;
	}

	// if id NOT exist in objectsById, then find object in parent ObjectContext
	return parent ? parent->findObject<esl::object::Interface::Object>(id) : nullptr;
}

void ObjectContext::addReference(const std::string& id, esl::object::Interface::Object& object) {
	logger.trace << "Adding object reference with id=\"" << id << "\"\n";

	if(objectRefsById.count(id) != 0) {
        throw std::runtime_error("Cannot add an object reference with id '" + id + "' because there exists already a object reference with same id.");
	}

	objectRefsById.insert(std::make_pair(id, std::ref(object)));
	//objectRefsById[id] = std::ref(object);
}

} /* namespace engine */
} /* namespace jerry */
