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

#include <jerry/engine/Applications.h>

namespace jerry {
namespace engine {

Applications::Applications(ObjectContext& aObjectContext)
: objectContext(aObjectContext)
{ }

Application& Applications::addApplication(const std::string& name) {
	std::unique_ptr<Application> applicationPtr(new Application(name, objectContext));
	Application& application = *applicationPtr;

	applications.insert(std::make_pair(name, std::move(applicationPtr)));

	return application;
}

const std::map<std::string, std::unique_ptr<Application>>& Applications::getApplications() const noexcept {
	return applications;
}

void Applications::initializeContext(esl::object::ObjectContext& objectContext) {
	for(auto& applicationEntry : applications) {
		applicationEntry.second->initializeContext();
	}
}

} /* namespace engine */
} /* namespace jerry */
