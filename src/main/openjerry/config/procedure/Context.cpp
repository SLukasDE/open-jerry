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

#include <openjerry/config/procedure/Context.h>
#include <openjerry/config/procedure/EntryImpl.h>

namespace openjerry {
namespace config {
namespace procedure {

void Context::save(std::ostream& oStream, std::size_t spaces) const {
	oStream << makeSpaces(spaces) << "<context";

	if(getRefId().empty()) {
		if(!getId().empty()) {
			oStream << " id=\"" << getId() << "\"";
		}
		if(getInherit()) {
			oStream << " inherit=\"true\"";
		}
		else {
			oStream << " inherit=\"false\"";
		}
		oStream << ">\n";

		saveEntries(oStream, spaces+2);

		oStream << makeSpaces(spaces) << "</context>\n";
	}
	else {
		oStream << " ref-id=\"" << getRefId() << "\"/>\n";
	}
}

void Context::install(engine::procedure::Context& engineContext) const {
	if(getRefId().empty()) {
		std::unique_ptr<engine::procedure::Context> context(new engine::procedure::Context(engineContext.getProcessRegistry()));
		engine::procedure::Context& contextRef = *context;

		if(getInherit()) {
			contextRef.ObjectContext::setParent(&engineContext);
		}

		if(getId().empty()) {
			engineContext.addContext(std::move(context));
		}
		else {
			engineContext.addObject(getId(), std::unique_ptr<esl::object::Object>(context.release()));
		}

		/* *****************
		 * install entries *
		 * *****************/
		installEntries(contextRef);
	}
	else {
		engineContext.addContext(getRefId());
	}
}

} /* namespace procedure */
} /* namespace config */
} /* namespace openjerry */
