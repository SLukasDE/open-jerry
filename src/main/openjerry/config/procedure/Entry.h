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

#ifndef OPENJERRY_CONFIG_PROCEDURE_ENTRY_H_
#define OPENJERRY_CONFIG_PROCEDURE_ENTRY_H_

#include <openjerry/config/Config.h>
#include <openjerry/engine/procedure/Context.h>

#include <string>
#include <ostream>

#include <tinyxml2.h>

namespace openjerry {
namespace config {
namespace procedure {

class Entry : public Config {
public:
	Entry(const Entry&) = delete;
	using Config::Config;

	virtual void save(std::ostream& oStream, std::size_t spaces) const = 0;
	virtual void install(engine::procedure::Context& engineHttpContext) const = 0;
};

} /* namespace procedure */
} /* namespace config */
} /* namespace openjerry */

#endif /* OPENJERRY_CONFIG_PROCEDURE_ENTRY_H_ */
