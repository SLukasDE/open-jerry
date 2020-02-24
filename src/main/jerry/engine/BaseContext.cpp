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

namespace jerry {
namespace engine {

namespace {
Logger logger("jerry::engine::BaseContext");
} /* anonymous namespace */

esl::object::parameter::Interface::Object& BaseContext::addObject(const std::string& id, const std::string& implementation) {
	logger.trace << "Adding object with id=\"" + id + "\" and implementation=\"" + implementation + "\"\n";

	std::unique_ptr<esl::object::parameter::Interface::Object> object = jerry::getModule().getInterface<esl::object::parameter::Interface>(implementation).createObject();
	if(!object) {
        throw std::runtime_error("Cannot create an object with id '" + id + "' for unknown implementation '" + implementation + "'.");
	}

	esl::object::parameter::Interface::Object* objectPtr = object.get();
	localObjectsById[id] = std::move(object);

	return *objectPtr;
}

esl::object::parameter::Interface::Object* BaseContext::getObject(const std::string& id) const {
	logger.trace << "Lookup object with id=\"" + id + "\"\n";

	auto localObjectsByIdIter = localObjectsById.find(id);
	if(localObjectsByIdIter == std::end(localObjectsById)) {
		logger.trace << "Object not found in BaseContext\n";
		return nullptr;
	}

	logger.trace << "Object found in BaseContext\n";
    return localObjectsByIdIter->second.get();
}

} /* namespace engine */
} /* namespace jerry */
