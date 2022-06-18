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

#ifndef JERRY_BUILTIN_OBJECT_APPLICATIONS_ENTRY_H_
#define JERRY_BUILTIN_OBJECT_APPLICATIONS_ENTRY_H_

#include <jerry/engine/http/RequestContext.h>

#include <esl/com/basic/server/RequestContext.h>
#include <esl/io/Input.h>
#include <esl/object/Context.h>
#include <esl/object/Interface.h>

#include <string>

namespace jerry {
namespace builtin {
namespace object {
namespace applications {

class Application;

class Entry {
public:
	virtual ~Entry() = default;

	virtual void initializeContext(Application& ownerContext) = 0;

	virtual esl::io::Input accept(engine::http::RequestContext& requestContext, const esl::object::Interface::Object* object) = 0;
	virtual esl::io::Input accept(esl::com::basic::server::RequestContext& requestContext, const esl::object::Interface::Object* object) = 0;
	virtual void procedureRun(esl::object::Context& objectContext, const esl::object::Interface::Object* object) = 0;
	virtual void procedureCancel() = 0;

	virtual void dumpTree(std::size_t depth) const = 0;
};

} /* namespace applications */
} /* namespace object */
} /* namespace builtin */
} /* namespace jerry */

#endif /* JERRY_BUILTIN_OBJECT_APPLICATIONS_ENTRY_H_ */
