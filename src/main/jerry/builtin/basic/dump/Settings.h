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

#ifndef JERRY_BUILTIN_BASIC_DUMP_SETTINGS_H_
#define JERRY_BUILTIN_BASIC_DUMP_SETTINGS_H_

#include <esl/object/Interface.h>

#include <jerry/builtin/Settings.h>

#include <string>
#include <set>

namespace jerry {
namespace builtin {
namespace basic {
namespace dump {

class Settings : public builtin::Settings {
public:
	static std::unique_ptr<esl::object::Interface::Object> create();

	void addSetting(const std::string& key, const std::string& value) override;

	void setShowContext(bool showContext) noexcept;
	bool getShowContext() const noexcept;

	void setShowContent(bool showContent) noexcept;
	bool getShowContent() const noexcept;

	const std::set<std::string>& getNotifiers() const noexcept;

private:
	bool showContext = true;
	bool showContent = false;
	std::set<std::string> notifiers;
};

} /* namespace dump */
} /* namespace basic */
} /* namespace builtin */
} /* namespace jerry */

#endif /* JERRY_BUILTIN_BASIC_DUMP_SETTINGS_H_ */
