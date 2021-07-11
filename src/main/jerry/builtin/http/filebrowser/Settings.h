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

#ifndef JERRY_BUILTIN_HTTP_FILEBROWSER_SETTINGS_H_
#define JERRY_BUILTIN_HTTP_FILEBROWSER_SETTINGS_H_

#include <esl/object/Interface.h>

#include <string>
#include <set>

namespace jerry {
namespace builtin {
namespace http {
namespace filebrowser {

class Settings : public esl::object::Interface::Object {
public:
	Settings(const esl::object::Interface::Settings& settings);

	bool isBrowsable() const;
	const std::string& getPath() const;
	const std::set<std::string>& getDefaults() const;
	bool getIgnoreError() const;

private:
	bool browsable = false;
	std::string path = "/";
	std::set<std::string> defaults;
	bool ignoreError = false;
};

} /* namespace filebrowser */
} /* namespace http */
} /* namespace builtin */
} /* namespace jerry */

#endif /* JERRY_BUILTIN_HTTP_FILEBROWSER_SETTINGS_H_ */
