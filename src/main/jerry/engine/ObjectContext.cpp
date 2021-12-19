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
#include <jerry/Logger.h>

#include <esl/object/InitializeContext.h>

#include <stdexcept>

namespace jerry {
namespace engine {

namespace {
Logger logger("jerry::engine::ObjectContext");
} /* anonymous namespace */

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
		if(initializeContext) {
			initializeContext->initializeContext(*this);
		}

		ObjectContext* objectContext = dynamic_cast<ObjectContext*>(object.second.get());
		if(objectContext) {
			objectContext->initializeContext();
		}
	}
}

void ObjectContext::dumpTree(std::size_t depth) const {
	for(const auto& objectEntry : objectRefsById) {
		for(std::size_t i=0; i<depth; ++i) {
			logger.info << "|   ";
		}

		const ObjectContext* context = dynamic_cast<const ObjectContext*>(&objectEntry.second.get());
		if(context) {
			logger.info << "+-> Context: \"";
		}
		else {
			logger.info << "+-> Object: \"";
		}
		if(objects.count(objectEntry.first) != 0) {
			logger.info << objectEntry.first << "\" -> " << &objectEntry.second.get() << "\n";
		}
		else {
			logger.info << objectEntry.first << "\" -> " << &objectEntry.second.get() << " (reference)\n";
		}
	}
}

esl::object::Interface::Object* ObjectContext::getObject() const {
	auto iter = objectRefsById.find("");
	return iter == std::end(objectRefsById) ? nullptr : &iter->second.get();
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

const std::map<std::string, std::reference_wrapper<esl::object::Interface::Object>>& ObjectContext::getObjects() const {
	return objectRefsById;
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
