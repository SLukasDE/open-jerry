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

#include <jerry/builtin/object/applications/xmlconfig/Procedure.h>

#include <esl/processing/Procedure.h>
#include <esl/utility/String.h>

namespace jerry {
namespace builtin {
namespace object {
namespace applications {
namespace xmlconfig {

void Procedure::install(Application& engineApplicatonContext) const {
	if(getRefId().empty()) {
		std::unique_ptr<esl::processing::Procedure> procedure = create();

		if(getId().empty()) {
			engineApplicatonContext.addProcedure(std::move(procedure));
		}
		else {
			engineApplicatonContext.addObject(getId(), std::unique_ptr<esl::object::Object>(procedure.release()));
		}
	}
	else {
		engineApplicatonContext.addProcedure(getRefId());
	}
}

} /* namespace xmlconfig */
} /* namespace applications */
} /* namespace object */
} /* namespace builtin */
} /* namespace jerry */
