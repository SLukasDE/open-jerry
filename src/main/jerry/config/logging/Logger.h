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

#ifndef JERRY_CONFIG_LOGGING_LOGGER_H_
#define JERRY_CONFIG_LOGGING_LOGGER_H_

#include <jerry/config/Config.h>
#include <jerry/config/logging/Appender.h>
#include <jerry/config/logging/Layout.h>
#include <jerry/config/logging/LevelSetting.h>

#include <tinyxml2/tinyxml2.h>

#include <boost/filesystem/path.hpp>

#include <map>
#include <ostream>
#include <string>
#include <vector>

namespace jerry {
namespace config {
namespace logging {

class Logger : public Config {
public:
	explicit Logger(const std::string& configuration);
	explicit Logger(const boost::filesystem::path& filename);
	Logger(const std::string& filename, const tinyxml2::XMLElement& element);

	void save(std::ostream& oStream) const;
	void install() const;

private:
	std::map<std::string, Layout> layouts;
	std::vector<Appender> appenders;
	std::vector<LevelSetting> settings;

	void loadXML(const tinyxml2::XMLElement& element);

	void parseInnerElement(const tinyxml2::XMLElement& element);
};

} /* namespace logging */
} /* namespace config */
} /* namespace jerry */

#endif /* JERRY_CONFIG_LOGGING_LOGGER_H_ */
