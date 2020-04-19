/*
 * This file is part of Jerry application server.
 * Copyright (C) 2020 Sven Lukas
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

#ifndef JERRY_CONFIG_JERRY_H_
#define JERRY_CONFIG_JERRY_H_

#include <jerry/config/Certificate.h>
#include <jerry/config/Logger.h>
#include <jerry/config/Object.h>
//#include <jerry/config/Parameter.h>
#include <jerry/config/Listener.h>
#include <jerry/engine/Engine.h>
#include <jerry/engine/Listener.h>
#include <jerry/engine/Endpoint.h>
#include <jerry/engine/Context.h>
#include <esl/logging/Layout.h>
#include <esl/module/Library.h>
#include <vector>
#include <set>
#include <string>
#include <memory>

namespace jerry {
namespace config {

class Jerry {
public:
	Jerry(const std::string& fileName);

	void loadLibraries();
	std::unique_ptr<esl::logging::Layout> getLayout() const;
	void setEngine(engine::Engine& engine) const;

	void print();

	std::string fileName;

	std::vector<std::pair<std::string, esl::module::Library*>> eslLibraries;
	std::vector<std::pair<std::string, esl::module::Library*>> libraries;
	std::vector<std::string> includes;
	std::vector<Certificate> certificates;
	Logger loggerConfig;
	std::vector<Object> objects;
	std::vector<Listener> listeners;

	std::set<std::string> filesLoaded;

private:
	void setEngineContext(engine::Context& engineContext, const Listener& configContext) const;
//	void setEngineEndpoint(engine::Endpoint& engineEndpoint, const Listener& configEndpoint) const;
	void setEngineListener(engine::Listener& engineListener, const Listener& configListener) const;
};

} /* namespace config */
} /* namespace jerry */

#endif /* JERRY_CONFIG_JERRY_H_ */
