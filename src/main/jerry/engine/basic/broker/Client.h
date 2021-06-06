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

#ifndef JERRY_ENGINE_BASIC_BROKER_CLIENT_H_
#define JERRY_ENGINE_BASIC_BROKER_CLIENT_H_

#include <jerry/engine/basic/server/Socket.h>

#include <esl/com/basic/broker/Client.h>
#include <esl/com/basic/broker/Interface.h>
#include <esl/com/basic/client/Interface.h>
#include <esl/com/basic/server/Interface.h>

#include <string>
#include <vector>
#include <utility>
#include <memory>

namespace jerry {
namespace engine {
namespace basic {
namespace broker {

class Client : public esl::com::basic::broker::Interface::Client {
public:
	Client(esl::object::ObjectContext& engineContext, const std::string& id, const std::string& brokers, const std::vector<std::pair<std::string, std::string>>& settings, const std::string& implementation);

	esl::com::basic::server::Interface::Socket& getSocket() override;
	std::unique_ptr<esl::com::basic::client::Interface::Connection> createConnection(std::vector<std::pair<std::string, std::string>> parameters) override;

	server::Socket& getServer() noexcept;
	esl::com::basic::broker::Interface::Client& getClient() noexcept;

	void dumpTree(std::size_t depth) const;

private:
	const std::string& getId() const noexcept;
	const std::string& getBrokers() const noexcept;
	const std::string& getImplementation() const noexcept;

	esl::com::basic::broker::Client client;
	server::Socket socket;

	esl::object::ObjectContext& engineContext;
	std::string id;
	std::string brokers;
	std::string implementation;
};

} /* namespace broker */
} /* namespace basic */
} /* namespace engine */
} /* namespace jerry */

#endif /* JERRY_ENGINE_BASIC_BROKER_CLIENT_H_ */
