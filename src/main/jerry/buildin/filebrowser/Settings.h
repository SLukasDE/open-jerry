/*
 * This file is part of Jerry application server.
 * Copyright (C) 2020 Sven Lukas
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

#ifndef JERRY_BUILDIN_FILEBROWSER_SETTINGS_H_
#define JERRY_BUILDIN_FILEBROWSER_SETTINGS_H_

#include <jerry/buildin/Settings.h>
#include <string>
#include <set>

namespace jerry {
namespace buildin {
namespace filebrowser {

class Settings : public buildin::Settings {
public:
	static std::unique_ptr<esl::object::Interface::Object> create();

	void addSetting(const std::string& key, const std::string& value) override;

	void setBrowsable(bool browsable);
	bool isBrowsable() const;

	void setPath(const std::string& path);
	const std::string& getPath() const;

	void addDefault(const std::string& file);
	const std::set<std::string>& getDefaults() const;

	void setHttpStatus(int httpStatus);
	const int getHttpStatus() const;

private:
	bool browsable = false;
	std::string path = "/";
	std::set<std::string> defaults;
	int httpStatus = 200;
};

} /* namespace filebrowser */
} /* namespace buildin */
} /* namespace jerry */

#endif /* JERRY_BUILDIN_FILEBROWSER_SETTINGS_H_ */
