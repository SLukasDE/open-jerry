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

#ifndef JERRY_BUILTIN_OBJECT_APPLICATIONS_APPLICATION_H_
#define JERRY_BUILTIN_OBJECT_APPLICATIONS_APPLICATION_H_

#include <jerry/builtin/object/applications/Entry.h>
#include <jerry/engine/ObjectContext.h>
#include <jerry/engine/basic/Context.h>
#include <jerry/engine/http/Context.h>
#include <jerry/engine/http/RequestContext.h>
#include <jerry/engine/procedure/Context.h>
#include <jerry/engine/ProcessRegistry.h>

#include <esl/com/basic/server/RequestContext.h>
#include <esl/io/Input.h>
#include <esl/object/Object.h>
#include <esl/processing/Procedure.h>

#include <memory>
#include <string>
#include <vector>

namespace jerry {
namespace builtin {
namespace object {
namespace applications {

class Application : public engine::ObjectContext {
public:
	Application(const std::string& name, engine::ProcessRegistry* processRegistry);

	const std::string& getName() const noexcept;

	void initializeContext() override;

	void addProcedure(std::unique_ptr<esl::processing::Procedure> procedure);
	void addProcedure(const std::string& refId);

	void addProcedureContext(std::unique_ptr<engine::procedure::Context> procedureContext);
	void addProcedureContext(const std::string& refId);

	void addBasicContext(std::unique_ptr<engine::basic::Context> basicContext);
	void addBasicContext(const std::string& refId);

	void addHttpContext(std::unique_ptr<engine::http::Context> httpContext);
	void addHttpContext(const std::string& refId);

	esl::io::Input accept(engine::http::RequestContext& requestContext, const esl::object::Object* object) const;
	esl::io::Input accept(esl::com::basic::server::RequestContext& requestContext, const esl::object::Object* object) const;
	void procedureRun(esl::object::Context& objectContext, const esl::object::Object* object) const;
	void procedureCancel() const;

private:
	const std::string name;
	std::vector<std::unique_ptr<Entry>> entries;
};

} /* namespace applications */
} /* namespace object */
} /* namespace builtin */
} /* namespace jerry */

#endif /* JERRY_BUILTIN_OBJECT_APPLICATIONS_APPLICATION_H_ */
