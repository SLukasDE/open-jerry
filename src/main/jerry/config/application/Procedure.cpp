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

#include <jerry/config/application/Procedure.h>
#include <jerry/config/XMLException.h>

#include <esl/processing/procedure/Interface.h>
#include <esl/utility/String.h>

#include <stdexcept>
namespace jerry {
namespace config {
namespace application {

void Procedure::install(engine::application::Context& engineApplicatonContext) const {
	if(getRefId().empty()) {
		std::unique_ptr<esl::processing::procedure::Interface::Procedure> procedure = create();

		if(getId().empty()) {
			engineApplicatonContext.getProcedureContext().addProcedure(std::move(procedure));
		}
		else {
			engineApplicatonContext.addObject(getId(), std::unique_ptr<esl::object::Interface::Object>(procedure.release()));
		}
	}
	else {
		engineApplicatonContext.getProcedureContext().addProcedure(getRefId());
	}
}

} /* namespace application */
} /* namespace config */
} /* namespace jerry */
