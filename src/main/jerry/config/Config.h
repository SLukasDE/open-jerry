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

#ifndef JERRY_CONFIG_CONFIG_H_
#define JERRY_CONFIG_CONFIG_H_

#include <jerry/config/Certificate.h>
#include <jerry/config/LoggerConfig.h>
#include <jerry/config/Object.h>
#include <jerry/config/Reference.h>
#include <jerry/config/OptionalBool.h>
#include <jerry/config/http/Server.h>
#include <jerry/config/http/Context.h>
#include <jerry/config/http/Listener.h>
#include <jerry/config/basic/Broker.h>
#include <jerry/config/basic/Server.h>
#include <jerry/config/basic/Context.h>
#include <jerry/config/basic/Listener.h>

#include <esl/logging/Layout.h>
#include <esl/module/Library.h>

#include <tinyxml2/tinyxml2.h>

#include <vector>
#include <set>
#include <string>
#include <memory>
#include <ostream>

namespace jerry {
namespace config {

class Config {
public:
	void loadFile(const std::string& fileName, const tinyxml2::XMLElement* element = nullptr);

	void loadLibraries();

	std::unique_ptr<esl::logging::Layout> createLayout() const;
	void setLogLevel() const;
	//void setEngine(engine::Engine& engine) const;

	void save(std::ostream& oStream) const;

	std::vector<std::pair<std::string, esl::module::Library*>> libraries;
	std::vector<std::string> includes;
	std::vector<Certificate> certificates;
	LoggerConfig loggerConfig;
	std::vector<Object> objects;

	std::vector<http::Server> httpServers;
	std::vector<http::Context> httpContext;
	std::vector<http::Listener> httpListeners;

	std::vector<basic::Broker> basicBrokers;
	std::vector<basic::Server> basicServers;
	std::vector<basic::Context> basicContext;
	std::vector<basic::Listener> basicListeners;

	std::set<std::string> filesLoaded;

private:
	void parseInclude(const tinyxml2::XMLElement& element);
	void parseLibrary(const tinyxml2::XMLElement& element);
};

} /* namespace config */
} /* namespace jerry */

#endif /* JERRY_CONFIG_CONFIG_H_ */
