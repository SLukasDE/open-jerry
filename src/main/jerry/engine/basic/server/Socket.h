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

#ifndef JERRY_ENGINE_BASIC_SERVER_SOCKET_H_
#define JERRY_ENGINE_BASIC_SERVER_SOCKET_H_

#include <jerry/engine/basic/server/Listener.h>

#include <esl/com/basic/server/Interface.h>
#include <esl/io/Input.h>
#include <esl/object/ObjectContext.h>
#include <esl/object/Interface.h>

#include <cstdint>
#include <string>
#include <vector>
#include <set>
#include <functional>
#include <memory>

namespace jerry {
namespace engine {
namespace basic {
namespace server {

class Socket final : public esl::com::basic::server::Interface::Socket {
public:
	Socket(esl::object::ObjectContext& engineContext, const std::string& id, esl::com::basic::server::Interface::Socket& socket);
	Socket(esl::object::ObjectContext& engineContext, const std::string& id,
			const esl::object::Interface::Settings& settings, const std::string& implementation);

	void addObjectFactory(const std::string& id, ObjectFactory objectFactory) override;

	void listen(const std::set<std::string>& notifications, esl::com::basic::server::requesthandler::Interface::CreateInput createInput) override;
	void release() override;
	bool wait(std::uint32_t ms) override;

	void dumpTree(std::size_t depth) const;

	void addListener(Listener& listener);

	esl::com::basic::server::Interface::Socket& getSocket() const noexcept;
	const std::string& getId() const noexcept;
	const std::string& getImplementation() const noexcept;

	std::set<std::string> getNotifier() const;

	static esl::io::Input createMessageHandler(esl::com::basic::server::RequestContext& baseRequestContext);

private:

	esl::object::ObjectContext& engineContext;
	std::unique_ptr<esl::com::basic::server::Interface::Socket> socketPtr;
	esl::com::basic::server::Interface::Socket& socket;

	const std::string id;
	const std::string implementation;
	const esl::object::Interface::Settings settings;

	Listener* listener = nullptr;
};

} /* namespace server */
} /* namespace basic */
} /* namespace engine */
} /* namespace jerry */

#endif /* JERRY_ENGINE_BASIC_SERVER_SOCKET_H_ */
