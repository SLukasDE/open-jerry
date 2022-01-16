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

#ifndef JERRY_CONFIG_APPLICATION_APPLICATION_H_
#define JERRY_CONFIG_APPLICATION_APPLICATION_H_

#include <jerry/config/Config.h>
#include <jerry/config/application/Entry.h>
#include <jerry/config/application/BasicListener.h>
#include <jerry/config/application/HttpListener.h>

#include <jerry/builtin/object/application/Application.h>

#include <esl/module/Library.h>

#include <tinyxml2/tinyxml2.h>

#include <vector>
#include <set>
#include <string>
#include <memory>
#include <ostream>

namespace jerry {
namespace config {
namespace application {

class Application : public jerry::config::Config {
public:
	Application(const std::string& fileName);

	void save(std::ostream& oStream) const;
	void install(builtin::object::application::Application& engineApplication);

private:
	std::vector<std::unique_ptr<Entry>> entries;
	tinyxml2::XMLDocument xmlDocument;
	std::unique_ptr<BasicListener> basicListener;
	std::unique_ptr<HttpListener> httpListener;

	void loadXML(const tinyxml2::XMLElement& element);

	void parseInnerElement(const tinyxml2::XMLElement& element);
};

} /* namespace application */
} /* namespace config */
} /* namespace jerry */

#endif /* JERRY_CONFIG_APPLICATION_APPLICATION_H_ */
