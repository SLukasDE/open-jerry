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

#ifndef JERRY_CONFIG_MAIN_ENGINE_H_
#define JERRY_CONFIG_MAIN_ENGINE_H_

#include <jerry/config/Config.h>
#include <jerry/config/Certificate.h>
#include <jerry/config/LoggerConfig.h>
#include <jerry/config/main/Entry.h>
#include <jerry/config/Object.h>
#include <jerry/config/OptionalBool.h>

#include <jerry/engine/Engine.h>

#include <esl/logging/appender/Interface.h>
#include <esl/logging/layout/Interface.h>
#include <esl/module/Library.h>

#include <tinyxml2/tinyxml2.h>

#include <vector>
#include <set>
#include <string>
#include <memory>
#include <ostream>

namespace jerry {
namespace config {
namespace main {

class Engine : public Config {
public:
	Engine(const std::string& fileName);

	void save(std::ostream& oStream) const;
	void loadLibraries();
	std::unique_ptr<esl::logging::layout::Interface::Layout> install(engine::Engine& engine, esl::logging::appender::Interface::Appender& appender1, esl::logging::appender::Interface::Appender& appenderMemBuffer);
	//void install(engine::Engine& engine);

private:
	tinyxml2::XMLDocument xmlDocument;

	std::vector<std::pair<std::string, esl::module::Library*>> libraries;
	std::vector<Certificate> certificates;
	LoggerConfig loggerConfig;

	std::vector<std::unique_ptr<Entry>> entries;

	std::set<std::string> filesLoaded;

	void loadXML(const tinyxml2::XMLElement& element);

	void parseInnerElement(const tinyxml2::XMLElement& element);
	void parseInclude(const tinyxml2::XMLElement& element);
	void parseLibrary(const tinyxml2::XMLElement& element);
};

} /* namespace main */
} /* namespace config */
} /* namespace jerry */

#endif /* JERRY_CONFIG_MAIN_ENGINE_H_ */