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

#include <openjerry/config/http/Procedure.h>

#include <esl/object/Object.h>
#include <esl/object/Procedure.h>
#include <esl/utility/String.h>

namespace openjerry {
namespace config {
namespace http {

void Procedure::install(engine::http::Context& engineHttpContext) const {
	if(getRefId().empty()) {
		std::unique_ptr<esl::object::Procedure> procedure = create();

		if(getId().empty()) {
			engineHttpContext.addProcedure(std::move(procedure));
		}
		else {
			engineHttpContext.addObject(getId(), std::unique_ptr<esl::object::Object>(procedure.release()));
		}
	}
	else {
		engineHttpContext.addProcedure(getRefId());
	}
}

} /* namespace http */
} /* namespace config */
} /* namespace openjerry */
