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

#ifndef JERRY_ENGINE_ENTRY_H_
#define JERRY_ENGINE_ENTRY_H_

#include <jerry/engine/basic/Socket.h>
#include <jerry/engine/http/Socket.h>

#include <esl/object/Interface.h>

#include <memory>

namespace jerry {
namespace engine {

struct Entry {
	Entry(std::unique_ptr<basic::Socket> basicServer);
	Entry(std::unique_ptr<http::Socket> httpServer);
	Entry(std::unique_ptr<esl::object::Interface::Object> object);

	std::unique_ptr<basic::Socket> basicServer;
	std::unique_ptr<http::Socket> httpServer;
	std::unique_ptr<esl::object::Interface::Object> object;
};

} /* namespace engine */
} /* namespace jerry */

#endif /* JERRY_ENGINE_ENTRY_H_ */
