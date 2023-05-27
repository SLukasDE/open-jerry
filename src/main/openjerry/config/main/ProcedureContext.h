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

#ifndef OPENJERRY_CONFIG_MAIN_PROCEDURECONTEXT_H_
#define OPENJERRY_CONFIG_MAIN_PROCEDURECONTEXT_H_

#include <openjerry/config/ProcedureContext.h>
#include <openjerry/engine/main/Context.h>

#include <ostream>
#include <string>

namespace openjerry {
namespace config {
namespace main {

class ProcedureContext : public config::ProcedureContext {
public:
	using config::ProcedureContext::ProcedureContext;

	void save(std::ostream& oStream, std::size_t spaces) const;
	void install(engine::main::Context& engineContext) const;
};

} /* namespace main */
} /* namespace config */
} /* namespace openjerry */

#endif /* OPENJERRY_CONFIG_MAIN_PROCEDURECONTEXT_H_ */
