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

#ifndef JERRY_BUILTIN_OBJECT_APPLICATIONS_XMLCONFIG_ENTRY_H_
#define JERRY_BUILTIN_OBJECT_APPLICATIONS_XMLCONFIG_ENTRY_H_

#include <jerry/config/Config.h>
#include <jerry/builtin/object/applications/Application.h>

#include <ostream>
#include <string>

namespace jerry {
namespace builtin {
namespace object {
namespace applications {
namespace xmlconfig {

class Entry : public config::Config {
public:
	Entry(const Entry&) = delete;
	using config::Config::Config;

	virtual void save(std::ostream& oStream, std::size_t spaces) const = 0;
	virtual void install(Application& engineApplicationContext) const = 0;
};

} /* namespace xmlconfig */
} /* namespace applications */
} /* namespace object */
} /* namespace builtin */
} /* namespace jerry */

#endif /* JERRY_BUILTIN_OBJECT_APPLICATIONS_XMLCONFIG_ENTRY_H_ */