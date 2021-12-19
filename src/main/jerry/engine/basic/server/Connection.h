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
#ifndef JERRY_ENGINE_BASIC_SERVER_CONNECTION_H_
#define JERRY_ENGINE_BASIC_SERVER_CONNECTION_H_

#include <jerry/engine/basic/server/Context.h>

#include <esl/com/basic/server/Connection.h>
//#include <esl/com/basic/server/Response.h>

#include <boost/filesystem.hpp>

namespace jerry {
namespace engine {
namespace basic {
namespace server {

class Connection : public esl::com::basic::server::Connection {
public:
	Connection(esl::com::basic::server::Connection& baseConnection);

	bool send(esl::io::Output output, std::vector<std::pair<std::string, std::string>> parameters) override;

	void setParent(const Context* context);

private:
	const Context* context = nullptr;
	esl::com::basic::server::Connection& baseConnection;
};

} /* namespace server */
} /* namespace basic */
} /* namespace engine */
} /* namespace jerry */

#endif /* JERRY_ENGINE_BASIC_SERVER_CONNECTION_H_ */
#endif

