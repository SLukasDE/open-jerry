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

#ifndef JERRY_CONFIG_LOGGING_APPENDER_H_
#define JERRY_CONFIG_LOGGING_APPENDER_H_

#include <jerry/config/Config.h>
#include <jerry/config/Setting.h>

#include <esl/logging/Appender.h>

#include <tinyxml2/tinyxml2.h>

#include <memory>
#include <ostream>
#include <string>
#include <vector>

namespace jerry {
namespace config {
namespace logging {

class Appender : public Config {
public:
	Appender(const std::string& fileName, const tinyxml2::XMLElement& element);

	const std::string& getName() const;
	const std::string& getLayoutId() const;

	void save(std::ostream& oStream, std::size_t spaces) const;

	std::unique_ptr<esl::logging::Appender> create() const;

private:
	std::string name;
	esl::logging::Appender::RecordLevel recordLevel = esl::logging::Appender::RecordLevel::SELECTED;
	std::string layoutId;
	std::string implementation;
	std::vector<Setting> parameters;

	void parseInnerElement(const tinyxml2::XMLElement& element);
};

} /* namespace logging */
} /* namespace config */
} /* namespace jerry */

#endif /* JERRY_CONFIG_LOGGING_APPENDER_H_ */
