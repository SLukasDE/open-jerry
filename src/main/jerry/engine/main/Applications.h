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

#ifndef JERRY_ENGINE_MAIN_APPLICATIONS_H_
#define JERRY_ENGINE_MAIN_APPLICATIONS_H_

#include <jerry/engine/ObjectContext.h>
#include <jerry/engine/application/Context.h>

#include <esl/object/ObjectContext.h>
#include <esl/object/InitializeContext.h>

#include <map>
#include <string>
#include <memory>

namespace jerry {
namespace engine {
namespace main {

class Applications : public esl::object::InitializeContext {
public:
	Applications(ObjectContext& objectContext);

	void addApplicationContext(const std::string& name, std::unique_ptr<application::Context> applicationContext);
	const std::map<std::string, std::unique_ptr<application::Context>>& getApplications() const noexcept;
	ObjectContext& getObjectContext() const noexcept;

	void initializeContext(esl::object::ObjectContext& objectContext) override;

private:
	ObjectContext& objectContext;
	std::map<std::string, std::unique_ptr<application::Context>> applications;
};

} /* namespace main */
} /* namespace engine */
} /* namespace jerry */

#endif /* JERRY_ENGINE_MAIN_APPLICATIONS_H_ */
