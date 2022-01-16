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

#ifndef JERRY_BUILTIN_OBJECT_APPLICATION_OBJECT_H_
#define JERRY_BUILTIN_OBJECT_APPLICATION_OBJECT_H_

#include <jerry/builtin/object/application/Application.h>

#include <esl/object/InitializeContext.h>
#include <esl/object/Interface.h>
#include <esl/object/ObjectContext.h>

#include <boost/filesystem.hpp>

#include <vector>
#include <string>
#include <memory>

namespace jerry {
namespace builtin {
namespace object {
namespace application {

class Object final : public esl::object::InitializeContext {
public:
	static inline const char* getImplementation() {
		return "jerry/applications";
	}

	static std::unique_ptr<esl::object::Interface::Object> create(const esl::module::Interface::Settings& settings);

	Object(const esl::module::Interface::Settings& settings);

	void initializeContext(esl::object::ObjectContext& objectContext) override;

	const std::vector<std::unique_ptr<Application>>& getApplications() const;
	//engine::basic::Context& getBasicContext() const;
	//engine::http::Context& getHttpContext() const;

private:
	boost::filesystem::path path;
	//ObjectContext objectContext;
	std::vector<std::unique_ptr<Application>> applications;
	//engine::basic::Context basicContext;
	//engine::http::Context httpContext;
};

} /* namespace application */
} /* namespace object */
} /* namespace builtin */
} /* namespace jerry */

#endif /* JERRY_BUILTIN_OBJECT_APPLICATION_OBJECT_H_ */
