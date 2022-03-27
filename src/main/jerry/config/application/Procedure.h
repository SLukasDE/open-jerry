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

#ifndef JERRY_CONFIG_APPLICATION_PROCEDURE_H_
#define JERRY_CONFIG_APPLICATION_PROCEDURE_H_

#include <jerry/config/Procedure.h>
#include <jerry/engine/application/Context.h>

namespace jerry {
namespace config {
namespace application {

class Procedure : public config::Procedure {
public:
	using config::Procedure::Procedure;

	void install(engine::application::Context& engineApplicatonContext) const;
};

} /* namespace application */
} /* namespace config */
} /* namespace jerry */

#endif /* JERRY_CONFIG_APPLICATION_PROCEDURE_H_ */
