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

#ifndef JERRY_ENGINE_ENGINE_H_
#define JERRY_ENGINE_ENGINE_H_

#include <jerry/engine/BaseContext.h>
#include <jerry/engine/messaging/broker/Client.h>
#include <jerry/engine/messaging/server/Socket.h>
#include <jerry/engine/messaging/server/Listener.h>
#include <jerry/engine/http/server/Socket.h>
#include <jerry/engine/http/server/Listener.h>

#include <esl/object/Interface.h>
#include <esl/utility/MessageTimer.h>

#include <cstdint>
#include <set>
#include <map>
#include <vector>
#include <string>
#include <utility>
#include <thread>
#include <functional>
#include <memory>

namespace jerry {
namespace engine {

class Engine : public BaseContext {
public:
	struct Message {
		int i;
		std::function<void(Engine& engine)> f;
	};

	Engine();

	bool run();
	bool runCGI();
	void stop();

	void addCertificate(const std::string& hostname, std::vector<unsigned char> key, std::vector<unsigned char> certificate);
	void addCertificate(const std::string& hostname, const std::string& keyFile, const std::string& certificateFile);

	void addMessageBroker(const std::string& id, const std::string& brokers, const std::vector<std::pair<std::string, std::string>>& settings, const std::string& implementation);
	void addMessageServer(const std::string& id, std::uint16_t port, const std::vector<std::pair<std::string, std::string>>& settings, const std::string& implementation);
	void addHttpServer(const std::string& id, std::uint16_t port, bool isHttps, const std::vector<std::pair<std::string, std::string>>& settings, const std::string& implementation);

	messaging::server::Listener& addMessageListener(const std::string& refId, bool inheritObjects);
	http::server::Listener& addHttpListener(const std::string& refId, bool inheritObjects, const std::string& hostname);

	void dumpTree(std::size_t depth) const override;

private:
	void dumpTreeMessageBrokers(std::size_t depth) const;
	void dumpTreeMessageServers(std::size_t depth) const;
	void dumpTreeHttpServers(std::size_t depth) const;
	void dumpTreeMessageListener(std::size_t depth) const;
	void dumpTreeHttpListener(std::size_t depth) const;

    std::thread::id runThreadId = std::thread::id();

    esl::utility::MessageTimer<std::string, Message> messageTimer;
	std::map<std::string, std::pair<std::vector<unsigned char>, std::vector<unsigned char>>> certsByHostname;

	/* *********************** *
	 * General listening ports *
	 * *********************** */
	std::set<std::uint16_t> listeningPorts;


	/* ***************** *
	 * Messaging Brokers *
	 * ***************** */
	std::map<std::string, std::reference_wrapper<messaging::broker::Client>> messageBrokerById;


	/* ***************** *
	 * Messaging Servers *
	 * ***************** */
	std::map<std::string, std::reference_wrapper<messaging::server::Socket>> messageServerById;


	/* ************ *
	 * HTTP Servers *
	 * ************ */
	std::map<std::string, std::reference_wrapper<http::server::Socket>> httpServerById;

	std::vector<std::unique_ptr<messaging::server::Listener>> messageListeners;
	std::vector<std::unique_ptr<http::server::Listener>> httpListeners;
};

} /* namespace engine */
} /* namespace jerry */

#endif /* JERRY_ENGINE_ENGINE_H_ */
