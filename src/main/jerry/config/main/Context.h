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

#ifndef JERRY_CONFIG_MAIN_CONTEXT_H_
#define JERRY_CONFIG_MAIN_CONTEXT_H_

#include <jerry/config/Config.h>
#include <jerry/config/Certificate.h>
#include <jerry/config/Object.h>
#include <jerry/config/OptionalBool.h>
#include <jerry/config/main/Entry.h>
#include <jerry/engine/main/Context.h>
#include <jerry/config/logging/Logger.h>

#include <esl/logging/appender/Interface.h>
#include <esl/logging/layout/Interface.h>
#include <esl/module/Library.h>

#include <tinyxml2/tinyxml2.h>

#include <boost/filesystem/path.hpp>

#include <memory>
#include <ostream>
#include <set>
#include <string>
#include <vector>

namespace jerry {
namespace config {
namespace main {

class Context : public Config {
public:
	explicit Context(const std::string& configuration, bool isJBoot = false);
	explicit Context(const boost::filesystem::path& filename, bool isJBoot = false);

	void save(std::ostream& oStream) const;
	void loadLibraries();

	void install(engine::main::Context& context);

private:
	tinyxml2::XMLDocument xmlDocument;
	bool isJBoot;

	std::vector<std::pair<std::string, esl::module::Library*>> libraries;
	std::vector<Certificate> certificates;

	std::vector<std::unique_ptr<Entry>> entries;

	std::set<std::string> filesLoaded;
	std::vector<logging::Logger> eslLoggers;

	void loadXML(const tinyxml2::XMLElement& element);

	void parseInnerElement(const tinyxml2::XMLElement& element);
	void parseInclude(const tinyxml2::XMLElement& element);
	void parseLibrary(const tinyxml2::XMLElement& element);
};

} /* namespace main */
} /* namespace config */
} /* namespace jerry */

#endif /* JERRY_CONFIG_MAIN_CONTEXT_H_ */