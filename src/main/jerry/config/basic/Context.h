/*
 * This file is part of Jerry application server.
 * Copyright (C) 2020-2021 Sven Lukas
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

#ifndef JERRY_CONFIG_BASIC_CONTEXT_H_
#define JERRY_CONFIG_BASIC_CONTEXT_H_

#include <jerry/config/Config.h>
#include <jerry/engine/Engine.h>
#include <jerry/engine/basic/server/Context.h>

#include <tinyxml2/tinyxml2.h>

#include <vector>
#include <ostream>

namespace jerry {
namespace config {
namespace basic {

class Entry;

class Context : public Config {
public:
	Context(const std::string& fileName, const tinyxml2::XMLElement& element, bool isGlobal);

	void save(std::ostream& oStream, std::size_t spaces) const;
	void install(engine::basic::server::Context& basicContext) const;
	void install(engine::Engine& engine) const;

private:
	const bool isGlobal;

	std::string id;
	std::string refId;

	bool inherit = true;
	std::vector<Entry> entries;

	void parseInnerElement(const tinyxml2::XMLElement& element);
};

} /* namespace basic */
} /* namespace config */
} /* namespace jerry */

#endif /* JERRY_CONFIG_BASIC_CONTEXT_H_ */
