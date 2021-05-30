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

#ifndef JERRY_ENGINE_MESSAGING_SERVER_SOCKET_H_
#define JERRY_ENGINE_MESSAGING_SERVER_SOCKET_H_

#include <jerry/engine/messaging/server/Listener.h>

#include <esl/messaging/server/Interface.h>
#include <esl/io/Input.h>
#include <esl/object/ObjectContext.h>

#include <cstdint>
#include <string>
#include <vector>
#include <set>
#include <functional>
#include <memory>

namespace jerry {
namespace engine {
namespace messaging {
namespace server {

class Socket final : public esl::messaging::server::Interface::Socket {
public:
	Socket(esl::object::ObjectContext& engineContext, const std::string& id, esl::messaging::server::Interface::Socket& socket);
	Socket(esl::object::ObjectContext& engineContext, const std::string& id,
			std::uint16_t port, const std::vector<std::pair<std::string, std::string>>& settings,
			const std::string& implementation);

	void addObjectFactory(const std::string& id, ObjectFactory objectFactory) override;

	void listen(const std::set<std::string>& notifications, esl::messaging::server::requesthandler::Interface::CreateInput createInput) override;
	void release() override;
	bool wait(std::uint32_t ms) override;

	void dumpTree(std::size_t depth) const;

	void addListener(Listener& listener);

	esl::messaging::server::Interface::Socket& getSocket() const noexcept;
	const std::string& getId() const noexcept;
	const std::string& getImplementation() const noexcept;
	std::uint16_t getPort() const noexcept;

	std::set<std::string> getNotifier() const;

	static esl::io::Input createMessageHandler(esl::messaging::server::RequestContext& baseRequestContext);

private:

	esl::object::ObjectContext& engineContext;
	std::unique_ptr<esl::messaging::server::Interface::Socket> socketPtr;
	esl::messaging::server::Interface::Socket& socket;

	std::string id;
	std::string implementation;
	std::uint16_t port = 0;

	Listener* listener = nullptr;
};

} /* namespace server */
} /* namespace messaging */
} /* namespace engine */
} /* namespace jerry */

#endif /* JERRY_ENGINE_MESSAGING_SERVER_SOCKET_H_ */
