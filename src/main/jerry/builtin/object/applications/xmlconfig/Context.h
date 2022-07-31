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

#ifndef JERRY_BUILTIN_OBJECT_APPLICATIONS_XMLCONFIG_CONTEXT_H_
#define JERRY_BUILTIN_OBJECT_APPLICATIONS_XMLCONFIG_CONTEXT_H_

#include <jerry/config/Config.h>
#include <jerry/builtin/object/applications/Application.h>
#include <jerry/builtin/object/applications/xmlconfig/Entry.h>
//#include <jerry/builtin/object/applications/Object.h>

#include <esl/plugin/Library.h>

#include <tinyxml2/tinyxml2.h>

#include <boost/filesystem.hpp>

#include <vector>
#include <set>
#include <string>
#include <memory>
#include <ostream>

namespace jerry {
namespace builtin {
namespace object {
namespace applications {
namespace xmlconfig {

class Context : public config::Config {
public:
	Context(const boost::filesystem::path& path);

	void save(std::ostream& oStream) const;
	void install(Application& application);

private:
	tinyxml2::XMLDocument xmlDocument;
	std::string appName;
	const boost::filesystem::path path;
	std::vector<std::pair<std::string, std::string>> libraries;

	std::vector<std::unique_ptr<Entry>> entries;

	void loadXML(const tinyxml2::XMLElement& element);

	void parseInnerElement(const tinyxml2::XMLElement& element);
	void parseLibrary(const tinyxml2::XMLElement& element);
	void loadLibraries();
};

} /* namespace xmlconfig */
} /* namespace applications */
} /* namespace object */
} /* namespace builtin */
} /* namespace jerry */

#endif /* JERRY_BUILTIN_OBJECT_APPLICATIONS_XMLCONFIG_CONTEXT_H_ */
