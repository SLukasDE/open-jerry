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

#ifndef OPENJERRY_CONFIG_MAIN_CONTEXT_H_
#define OPENJERRY_CONFIG_MAIN_CONTEXT_H_

#include <openjerry/config/Config.h>
#include <openjerry/config/Certificate.h>
#include <openjerry/config/Object.h>
#include <openjerry/config/OptionalBool.h>
#include <openjerry/config/main/Entry.h>
#include <openjerry/engine/main/Context.h>
//#include <openjerry/config/logging/Logger.h>

#include <esl/plugin/Library.h>

#include <tinyxml2/tinyxml2.h>

#include <boost/filesystem/path.hpp>

#include <memory>
#include <ostream>
#include <set>
#include <string>
#include <vector>

namespace openjerry {
namespace config {
namespace main {

class Context : public Config {
public:
	explicit Context(const std::string& configuration);
	explicit Context(const boost::filesystem::path& filename);

	void save(std::ostream& oStream) const;
	void loadLibraries();

	void install(engine::main::Context& context);

private:
	tinyxml2::XMLDocument xmlDocument;

	std::vector<std::pair<std::string, std::string>> libraries;
	//std::vector<Certificate> certificates;

	std::vector<std::unique_ptr<Entry>> entries;

	std::set<std::string> filesLoaded;
	//std::vector<logging::Logger> eslLoggers;

	void loadXML(const tinyxml2::XMLElement& element);

	void parseInnerElement(const tinyxml2::XMLElement& element);
	void parseInclude(const tinyxml2::XMLElement& element);
	void parseLibrary(const tinyxml2::XMLElement& element);
};

} /* namespace main */
} /* namespace config */
} /* namespace openjerry */

#endif /* OPENJERRY_CONFIG_MAIN_CONTEXT_H_ */
