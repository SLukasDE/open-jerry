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

#ifndef JERRY_CONFIG_LOGGING_LEVELSETTING_H_
#define JERRY_CONFIG_LOGGING_LEVELSETTING_H_

#include <jerry/config/Config.h>

#include <esl/logging/Level.h>

#include <tinyxml2/tinyxml2.h>

#include <string>
#include <ostream>

namespace jerry {
namespace config {
namespace logging {

class LevelSetting : public Config {
public:
	LevelSetting(const std::string& fileName, const tinyxml2::XMLElement& element);

	esl::logging::Level getLevel() const;
	const std::string& getScope() const;

	void save(std::ostream& oStream, std::size_t spaces) const;

private:
	esl::logging::Level level = esl::logging::Level::SILENT;
	std::string scope;
};

} /* namespace logging */
} /* namespace config */
} /* namespace jerry */

#endif /* JERRY_CONFIG_LOGGING_LEVELSETTING_H_ */
