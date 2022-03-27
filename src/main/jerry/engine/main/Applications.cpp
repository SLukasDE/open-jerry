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

#include <jerry/engine/main/Applications.h>

namespace jerry {
namespace engine {
namespace main {

Applications::Applications(ObjectContext& aObjectContext)
: objectContext(aObjectContext)
{ }

void Applications::addApplicationContext(const std::string& name, std::unique_ptr<application::Context> applicationContext) {
	applications.insert(std::make_pair(name, std::move(applicationContext)));
}

const std::map<std::string, std::unique_ptr<application::Context>>& Applications::getApplications() const noexcept {
	return applications;
}

ObjectContext& Applications::getObjectContext() const noexcept {
	return objectContext;
}

void Applications::initializeContext(esl::object::ObjectContext& objectContext) {
	for(auto& applicationEntry : applications) {
		applicationEntry.second->initializeContext();
	}
}

} /* namespace main */
} /* namespace engine */
} /* namespace jerry */
