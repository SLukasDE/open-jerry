#if 0
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

#include <jerry/Logger.h>

#include <esl/logging/Config.h>

#include <iostream>
#include <map>
#include <memory>
#include <stdexcept>
#include <sstream>
#include <string>
#include <utility>
#include <vector>

namespace jerry {

namespace {
std::map<std::string, std::unique_ptr<esl::logging::Layout>> layouts;
std::vector<std::pair<std::string, std::unique_ptr<esl::logging::Appender>>> appenders;
}

void Logger::flush() {
	for(auto& appender : appenders) {
		std::stringstream strStream;

		appender.second->flush();

		appender.second->flush(strStream);
		if(!strStream.str().empty()) {
			std::cerr << "\n\nFlush log messages from appender \"" << appender.first << "\":\n";
			std::cerr << strStream.str();
		}
	}
}

void Logger::addLayout(const std::string& id, std::unique_ptr<esl::logging::Layout> layout) {
	if(layouts.insert(std::make_pair(id, std::move(layout))).second == false) {
		throw std::runtime_error("Cannot add layout with id \"" + id + "\" because it exists already");
	}
}

void Logger::addAppender(const std::string& name, const std::string& layoutRefId, std::unique_ptr<esl::logging::Appender> appender) {
	auto iter = layouts.find(layoutRefId);
	if(iter == std::end(layouts)) {
		throw std::runtime_error("Appender is referencing an undefined layout \"" + layoutRefId + "\"");
	}

	appender->setLayout(iter->second.get());

    esl::logging::Config::addAppender(*appender);
    appenders.push_back(std::make_pair(name, std::move(appender)));
}

} /* namespace jerry */

#endif
