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

#ifndef JERRY_CONFIG_BASIC_ENTRY_H_
#define JERRY_CONFIG_BASIC_ENTRY_H_

#include <jerry/config/Config.h>
#include <jerry/engine/basic/server/Context.h>

#include <tinyxml2/tinyxml2.h>

#include <string>
#include <ostream>

namespace jerry {
namespace config {
namespace basic {

class Entry : public Config {
public:
	Entry(const Entry&) = delete;
	using Config::Config;

	virtual void save(std::ostream& oStream, std::size_t spaces) const = 0;
	virtual void install(engine::basic::server::Context& engineBasicContext) const = 0;
};

} /* namespace basic */
} /* namespace config */
} /* namespace jerry */

#endif /* JERRY_CONFIG_BASIC_ENTRY_H_ */
