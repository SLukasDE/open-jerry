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

#ifndef JERRY_ENGINE_BASIC_SERVER_SOCKET_H_
#define JERRY_ENGINE_BASIC_SERVER_SOCKET_H_

#include <jerry/engine/basic/server/Context.h>
#include <jerry/engine/basic/server/RequestHandler.h>

#include <esl/com/basic/server/Interface.h>
#include <esl/com/basic/server/Socket.h>
#include <esl/io/Input.h>
#include <esl/object/Interface.h>

#include <cstdint>
#include <string>
#include <vector>
#include <set>
#include <memory>
#include <functional>

namespace jerry {
namespace engine {
namespace basic {
namespace server {

class Socket final : public esl::com::basic::server::Interface::Socket {
public:
	Socket(const esl::object::Interface::Settings& settings, const std::string& implementation);

	void listen(std::function<void()> onReleasedHandler);
	std::set<std::string> getNotifiers() const;
	Context& getContext() noexcept;

	void listen(const esl::com::basic::server::requesthandler::Interface::RequestHandler& requestHandler, std::function<void()> onReleasedHandler) override;
	void release() override;
	bool wait(std::uint32_t ms) override;

	void initializeContext();

	void dumpTree(std::size_t depth) const;

private:
	esl::com::basic::server::Socket socket;
	RequestHandler requestHandler;

	const std::string implementation;
	const esl::object::Interface::Settings settings;

	Context context;
};

} /* namespace server */
} /* namespace basic */
} /* namespace engine */
} /* namespace jerry */

#endif /* JERRY_ENGINE_BASIC_SERVER_SOCKET_H_ */
