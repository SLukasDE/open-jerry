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
#include <jerry/engine/basic/broker/Client.h>
#include <jerry/engine/basic/server/Socket.h>
#include <jerry/engine/basic/server/Listener.h>
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

	bool run(bool isDaemon);
	bool runCGI();
	void stop();

	void addCertificate(const std::string& hostname, std::vector<unsigned char> key, std::vector<unsigned char> certificate);
	void addCertificate(const std::string& hostname, const std::string& keyFile, const std::string& certificateFile);

	void addBasicBroker(const std::string& id, const std::vector<std::pair<std::string, std::string>>& settings, const std::string& implementation);
	void addBasicServer(const std::string& id, const std::vector<std::pair<std::string, std::string>>& settings, const std::string& implementation);
	void addHttpServer(const std::string& id, bool isHttps, const std::vector<std::pair<std::string, std::string>>& settings, const std::string& implementation);

	basic::server::Listener& addBasicListener(const std::string& refId, bool inheritObjects);
	http::server::Listener& addHttpListener(const std::string& refId, bool inheritObjects, const std::string& hostname);

	void dumpTree(std::size_t depth) const override;

private:
	void dumpTreeBasicBrokers(std::size_t depth) const;
	void dumpTreeBasicServers(std::size_t depth) const;
	void dumpTreeHttpServers(std::size_t depth) const;
	void dumpTreeBasicListener(std::size_t depth) const;
	void dumpTreeHttpListener(std::size_t depth) const;

    std::thread::id runThreadId = std::thread::id();

    esl::utility::MessageTimer<std::string, Message> messageTimer;
	std::map<std::string, std::pair<std::vector<unsigned char>, std::vector<unsigned char>>> certsByHostname;


	/* ***************** *
	 * Messaging Brokers *
	 * ***************** */
	std::map<std::string, std::reference_wrapper<basic::broker::Client>> basicBrokerById;


	/* ***************** *
	 * Messaging Servers *
	 * ***************** */
	std::map<std::string, std::reference_wrapper<basic::server::Socket>> basicServerById;


	/* ************ *
	 * HTTP Servers *
	 * ************ */
	std::map<std::string, std::reference_wrapper<http::server::Socket>> httpServerById;

	std::vector<std::unique_ptr<basic::server::Listener>> basicListeners;
	std::vector<std::unique_ptr<http::server::Listener>> httpListeners;
};

} /* namespace engine */
} /* namespace jerry */

#endif /* JERRY_ENGINE_ENGINE_H_ */
