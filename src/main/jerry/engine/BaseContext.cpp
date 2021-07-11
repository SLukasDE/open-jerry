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

#include <jerry/engine/BaseContext.h>
#include <jerry/Logger.h>

#include <esl/Module.h>
#include <esl/object/InitializeContext.h>

namespace jerry {
namespace engine {

namespace {
Logger logger("jerry::engine::BaseContext");
} /* anonymous namespace */

bool BaseContext::hasObjectImplementation(const std::string& implementation) {
	return esl::getModule().findInterface<esl::object::Interface>(implementation) != nullptr;
}

esl::object::Interface::Object& BaseContext::addObject(const std::string& id, const std::string& implementation, const esl::object::Interface::Settings& settings) {
	logger.trace << "Adding object with id=\"" << id << "\" and implementation=\"" << implementation << "\"\n";

	if(objectsById.find(id) != std::end(objectsById)) {
        throw std::runtime_error("Cannot create an object with id '" + id + "' for implementation '" + implementation + "' because there exists already a local object with same id.");
	}

	std::unique_ptr<esl::object::Interface::Object> object = esl::getModule().getInterface<esl::object::Interface>(implementation).createObject(settings);
	if(!object) {
		throw std::runtime_error("Cannot create an object with id '" + id + "' for implementation '" + implementation + "' because interface method createObject() returns nullptr.");
	}

	esl::object::Interface::Object* objectPtr = object.get();
	objectsById[id] = std::move(object);

	return *objectPtr;
}

esl::object::Interface::Object& BaseContext::addObject(const std::string& id, std::unique_ptr<esl::object::Interface::Object> object) {
	logger.trace << "Adding object with id=\"" << id << "\"\n";

	if(objectsById.find(id) != std::end(objectsById)) {
        throw std::runtime_error("Cannot create an object with id '" + id + "' because there exists already a local object with same id.");
	}

	if(!object) {
		throw std::runtime_error("Cannot add an empty object with id '" + id + "'.");
	}

	esl::object::Interface::Object* objectPtr = object.get();
	objectsById[id] = std::move(object);

	return *objectPtr;
}

esl::object::Interface::Object* BaseContext::findObject(const std::string& id) const {
	auto localObjectsByIdIter = objectsById.find(id);
	if(localObjectsByIdIter == std::end(objectsById)) {
		return nullptr;
	}

    return localObjectsByIdIter->second.get();
}

void BaseContext::initializeContext() {
	for(auto& object : objectsById) {
		esl::object::InitializeContext* initializeContext = dynamic_cast<esl::object::InitializeContext*>(object.second.get());
		if(initializeContext) {
			initializeContext->initializeContext(*this);
		}
	}
}

void BaseContext::dumpTree(std::size_t depth) const {
	for(const auto& objectEntry : objectsById) {
		for(std::size_t i=0; i<depth; ++i) {
			logger.info << "|   ";
		}
		logger.info << "+-> Id: \"" << objectEntry.first << "\" -> " << objectEntry.second.get() << "\n";
	}
}

} /* namespace engine */
} /* namespace jerry */
