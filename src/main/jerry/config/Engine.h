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

#ifndef JERRY_CONFIG_ENGINE_H_
#define JERRY_CONFIG_ENGINE_H_

#include <jerry/engine/Engine.h>
#include <jerry/config/Config.h>

#include <string>

namespace jerry {
namespace config {

class Engine {
public:
	Engine(engine::Engine& engine);

	void install(const Config& config);
	static std::string evaluate(const std::string& expression, const std::string& language);

private:
	engine::Engine& engine;
};

} /* namespace config */
} /* namespace jerry */

#endif /* JERRY_CONFIG_ENGINE_H_ */
