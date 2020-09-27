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

#include <jerry/engine/BaseContext.h>
#include <jerry/Module.h>
#include <jerry/Logger.h>

#include <esl/http/server/InitializeContext.h>

namespace jerry {
namespace engine {

namespace {
Logger logger("jerry::engine::BaseContext");
} /* anonymous namespace */

esl::object::Interface::Object& BaseContext::addObject(const std::string& id, const std::string& implementation) {
	logger.trace << "Adding object with id=\"" << id << "\" and implementation=\"" << implementation << "\"\n";

	if(objectsById.find(id) != std::end(objectsById)) {
        throw std::runtime_error("Cannot create an object with id '" + id + "' for implementation '" + implementation + "' because there exists already a local object with same id.");
	}

	std::unique_ptr<esl::object::Interface::Object> object = jerry::getModule().getInterface<esl::object::Interface>(implementation).createObject();
	if(!object) {
		throw std::runtime_error("Cannot create an object with id '" + id + "' for implementation '" + implementation + "' because interface method createObject() returns nullptr.");
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
		esl::http::server::InitializeContext* initializeContext = dynamic_cast<esl::http::server::InitializeContext*>(object.second.get());
		if(initializeContext) {
			initializeContext->initializeContext(*this);
		}
	}
}

void BaseContext::dumpTree(std::size_t depth) const {
	for(const auto& objectEntry : objectsById) {
		for(std::size_t i=0; i<depth; ++i) {
			logger.info << "|  ";
		}
		logger.info << "+-> Id: \"" << objectEntry.first << "\" -> " << objectEntry.second.get() << "\n";
	}
}

} /* namespace engine */
} /* namespace jerry */
