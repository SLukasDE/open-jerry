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

#include <jerry/builtin/object/applications/xmlconfig/BasicContext.h>
#include <jerry/config/basic/RequestHandler.h>
#include <jerry/config/basic/EntryImpl.h>
#include <jerry/config/Object.h>
#include <jerry/engine/basic/Context.h>

#include <esl/utility/String.h>

#include <stdexcept>

namespace jerry {
namespace builtin {
namespace object {
namespace applications {
namespace xmlconfig {

void BasicContext::install(Application& engineApplicationContext) const {
	if(getRefId().empty()) {
		std::unique_ptr<engine::basic::Context> context(new engine::basic::Context(engineApplicationContext.getProcessRegistry()));
		engine::basic::Context& contextRef = *context;

		if(getInherit()) {
			contextRef.ObjectContext::setParent(&engineApplicationContext);
		}

		if(getId().empty()) {
			engineApplicationContext.addBasicContext(std::move(context));
		}
		else {
			engineApplicationContext.addObject(getId(), std::unique_ptr<esl::object::Object>(context.release()));
		}

		/* *****************
		 * install entries *
		 * *****************/
		installEntries(contextRef);
	}
	else {
		engineApplicationContext.addBasicContext(getRefId());
	}
}

} /* namespace xmlconfig */
} /* namespace applications */
} /* namespace object */
} /* namespace builtin */
} /* namespace jerry */
