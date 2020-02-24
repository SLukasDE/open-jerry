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

#ifndef JERRY_CONFIG_LISTENER_H_
#define JERRY_CONFIG_LISTENER_H_

#include <jerry/config/ListenerType.h>
#include <jerry/config/Reference.h>
#include <jerry/config/Object.h>
#include <string>
#include <vector>

namespace jerry {
namespace config {

struct Listener {
	ListenerType listenerType = ListenerType::listener;
	std::string url; // only if it is a Listener
	std::string path; // only if it is an Endpoint

	std::vector<Object> objects;
	std::vector<Reference> references; // only if it is an Endpoint or a Context
	std::vector<std::string> requesthandlers;
	std::vector<Listener> contextEndpoints;
};

} /* namespace config */
} /* namespace jerry */

#endif /* JERRY_CONFIG_LISTENER_H_ */
