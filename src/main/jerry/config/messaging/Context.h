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

#ifndef JERRY_CONFIG_MESSAGING_CONTEXT_H_
#define JERRY_CONFIG_MESSAGING_CONTEXT_H_

#include <jerry/config/messaging/Entry.h>

#include <tinyxml2/tinyxml2.h>

#include <vector>
#include <ostream>

namespace jerry {
namespace config {
namespace messaging {

struct Context {
	Context(const tinyxml2::XMLElement& element, bool hasEndpoint, bool isGlobal);

	void save(std::ostream& oStream, std::size_t spaces) const;

	const bool hasEndpoint;
	const bool isGlobal;

	bool hasId = false;
	std::string id;

	bool hasRefId = false;
	std::string refId;

	std::vector<Entry> entries;
};

} /* namespace messaging */
} /* namespace config */
} /* namespace jerry */

#endif /* JERRY_CONFIG_MESSAGING_CONTEXT_H_ */
