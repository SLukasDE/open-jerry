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

#include <jerry/ObjectContext.h>

namespace jerry {

void ObjectContext::addObject(const std::string& id, std::unique_ptr<esl::object::Interface::Object> object) {
	objects[id] = std::move(object);
}

esl::object::Interface::Object* ObjectContext::findRawObject(const std::string& id) {
	auto iter = objects.find(id);
	return iter == std::end(objects) ? nullptr : iter->second.get();
}

const esl::object::Interface::Object* ObjectContext::findRawObject(const std::string& id) const {
	auto iter = objects.find(id);
	return iter == std::end(objects) ? nullptr : iter->second.get();
}

} /* namespace jerry */
