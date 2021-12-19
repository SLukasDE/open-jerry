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

#if 0
#include <jerry/engine/basic/server/Connection.h>

namespace jerry {
namespace engine {
namespace basic {
namespace server {

Connection::Connection(esl::com::basic::server::Connection& aBaseConnection)
: baseConnection(aBaseConnection)
{ }

bool Connection::send(esl::io::Output output, std::vector<std::pair<std::string, std::string>> parameters) {
	return baseConnection.send(std::move(output), std::move(parameters));
}

void Connection::setParent(const Context* aContext) {
	context = aContext;
}

} /* namespace server */
} /* namespace basic */
} /* namespace engine */
} /* namespace jerry */
#endif
