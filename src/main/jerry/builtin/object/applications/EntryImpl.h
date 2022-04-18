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


#ifndef JERRY_BUILTIN_OBJECT_APPLICATIONS_ENTRYIMPL_H_
#define JERRY_BUILTIN_OBJECT_APPLICATIONS_ENTRYIMPL_H_

#include <jerry/builtin/object/applications/Entry.h>
#include <jerry/builtin/object/applications/Application.h>
#include <jerry/engine/main/Context.h>
#include <jerry/engine/basic/Server.h>
#include <jerry/engine/http/Server.h>
#include <jerry/engine/procedure/Context.h>
#include <jerry/engine/http/RequestContext.h>

#include <esl/com/basic/server/RequestContext.h>
#include <esl/io/Input.h>
#include <esl/object/Interface.h>
#include <esl/object/ObjectContext.h>
#include <esl/processing/procedure/Interface.h>

#include <memory>

namespace jerry {
namespace builtin {
namespace object {
namespace applications {

class EntryImpl : public Entry {
public:
	EntryImpl(std::unique_ptr<esl::processing::procedure::Interface::Procedure> procedure);
	EntryImpl(esl::processing::procedure::Interface::Procedure& refProcedure);

	EntryImpl(std::unique_ptr<engine::procedure::Context> procedureContext);
	EntryImpl(engine::procedure::Context& refProcedureContext);

	EntryImpl(std::unique_ptr<engine::basic::Context> basicContext);
	EntryImpl(engine::basic::Context& refBasicContext);

	EntryImpl(std::unique_ptr<engine::http::Context> httpContext);
	EntryImpl(engine::http::Context& refHttpContext);

	void initializeContext(Application& ownerContext) override;

	esl::io::Input accept(engine::http::RequestContext& requestContext, const esl::object::Interface::Object* object) override;
	esl::io::Input accept(esl::com::basic::server::RequestContext& requestContext, const esl::object::Interface::Object* object) override;
	void procedureRun(esl::object::ObjectContext& objectContext, const esl::object::Interface::Object* object) override;
	void procedureCancel() override;

	void dumpTree(std::size_t depth) const override;

private:
	std::unique_ptr<esl::processing::procedure::Interface::Procedure> procedure;
	esl::processing::procedure::Interface::Procedure* refProcedure = nullptr;

	std::unique_ptr<engine::procedure::Context> procedureContext;
	engine::procedure::Context* refProcedureContext = nullptr;

	std::unique_ptr<engine::basic::Context> basicContext;
	engine::basic::Context* refBasicContext = nullptr;

	std::unique_ptr<engine::http::Context> httpContext;
	engine::http::Context* refHttpContext = nullptr;
};

} /* namespace applications */
} /* namespace object */
} /* namespace builtin */
} /* namespace jerry */

#endif /* JERRY_BUILTIN_OBJECT_APPLICATIONS_ENTRYIMPL_H_ */
