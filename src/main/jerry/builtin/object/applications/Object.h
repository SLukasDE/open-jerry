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

#ifndef JERRY_BUILTIN_OBJECT_APPLICATIONS_OBJECT_H_
#define JERRY_BUILTIN_OBJECT_APPLICATIONS_OBJECT_H_

#include <jerry/builtin/object/applications/Application.h>
#include <jerry/engine/ObjectContext.h>

#include <esl/com/http/server/RequestContext.h>
#include <esl/com/basic/server/RequestContext.h>
#include <esl/io/Input.h>
#include <esl/object/Context.h>
#include <esl/object/InitializeContext.h>
#include <esl/object/Interface.h>

#include <map>
#include <memory>
#include <set>
#include <string>
#include <utility>
#include <vector>

namespace jerry {
namespace builtin {
namespace object {
namespace applications {

class Object : public esl::object::InitializeContext {
public:
	static inline const char* getImplementation() {
		return "jerry/applications";
	}

	static std::unique_ptr<esl::object::Interface::Object> create(const std::vector<std::pair<std::string, std::string>>& settings);

	Object(const std::vector<std::pair<std::string, std::string>>& settings);

	const std::map<std::string, std::unique_ptr<Application>>& getApplications() const noexcept;
	//engine::ObjectContext& getObjectContext() const noexcept;

	void initializeContext(esl::object::Context& objectContext) override;

	esl::io::Input accept(esl::com::http::server::RequestContext& requestContext, const Application* application, const esl::object::Interface::Object* object) const;

	esl::io::Input accept(esl::com::basic::server::RequestContext& requestContext, const Application* application, const esl::object::Interface::Object* object) const;
	std::set<std::string> getNotifiers() const;

	void procedureRun(esl::object::Context& objectContext, const Application* application, const esl::object::Interface::Object* object) const;
	void procedureCancel() const;

private:
	std::string path;
	engine::ObjectContext* objectContext = nullptr;
	//std::unique_ptr<engine::ObjectContext> objectContext;
	//std::unique_ptr<esl::object::Context> objectContext;
	std::map<std::string, std::unique_ptr<Application>> applications;

	void scan();
};

} /* namespace applications */
} /* namespace object */
} /* namespace builtin */
} /* namespace jerry */

#endif /* JERRY_BUILTIN_OBJECT_APPLICATIONS_OBJECT_H_ */
