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

#include <jerry/engine/application/Context.h>
#include <jerry/engine/ProcessRegistry.h>

namespace jerry {
namespace engine {
namespace application {

Context::Context(const std::string& aName, ObjectContext& parentObjectContext)
: ObjectContext(parentObjectContext.getProcessRegistry()),
  name(aName),
  basicContext(getProcessRegistry()),
  httpContext(getProcessRegistry()),
  procedureContext(getProcessRegistry())
{
	setParent(&parentObjectContext);

	basicContext.setParent(this);
	httpContext.ObjectContext::setParent(this);
	procedureContext.setParent(this);
}

const std::string& Context::getName() const noexcept {
	return name;
}
void Context::initializeContext() {
	ObjectContext::initializeContext();
	getBasicContext().initializeContext();
	getHttpContext().initializeContext();
	getProcedureContext().initializeContext();
}

basic::Context& Context::getBasicContext() noexcept {
	return basicContext;
}

http::Context& Context::getHttpContext() noexcept {
	return httpContext;
}

procedure::Context& Context::getProcedureContext() noexcept {
	return procedureContext;
}

} /* namespace application */
} /* namespace engine */
} /* namespace jerry */
