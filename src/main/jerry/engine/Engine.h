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
#include <jerry/engine/http/Listener.h>
#include <jerry/engine/messaging/proxy/Client.h>
#include <jerry/engine/messaging/Listener.h>
#include <jerry/engine/messaging/Endpoint.h>
#include <jerry/engine/messaging/Context.h>

#include <esl/messaging/Client.h>
#include <esl/messaging/messagehandler/Interface.h>
#include <esl/http/server/Socket.h>
#include <esl/http/server/requesthandler/Interface.h>
#include <esl/http/server/RequestContext.h>
#include <esl/object/Interface.h>
#include <esl/object/ValueSettings.h>
#include <esl/utility/MessageTimer.h>

#include <string>
#include <vector>
#include <map>
#include <set>
#include <utility>
#include <thread>
#include <functional>
#include <memory>
#include <cstdint>

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

	void addMessageBroker(const std::string& id, const std::string& brokers, std::uint16_t threads, const std::vector<std::pair<std::string, std::string>>& settings, const std::string& implementation);
	messaging::Listener& addMessageListener(const std::string& refId, bool inheritObjects);

	void addHttpServer(const std::string& id, std::uint16_t port, bool isHttps, const std::vector<std::pair<std::string, std::string>>& settings, const std::string& implementation);
	http::Listener& addHttpListener(const std::string& hostname, const std::string& refId, bool inheritObjects);

	void dumpTree(std::size_t depth) const override;
	void dumpTreeMessageBrokers(std::size_t depth) const;
	void dumpTreeHttpServers(std::size_t depth, bool isHttps) const;

private:

    std::thread::id runThreadId = std::thread::id();

    esl::utility::MessageTimer<std::string, Message> messageTimer;
	std::map<std::string, std::pair<std::vector<unsigned char>, std::vector<unsigned char>>> certsByHostname;

	/* *********************** *
	 * General listening ports *
	 * *********************** */

	std::set<std::uint16_t> listeningPorts;


	/* ******************* *
	 * Messaging variables *
	 * ******************* */

	std::map<std::string, std::reference_wrapper<messaging::proxy::Client>> messageBrokerById;

	std::map<std::string, std::unique_ptr<messaging::Listener>> messageListenerByBroker;


	/* ************** *
	 * HTTP variables *
	 * ************** */

	class HttpObject : public esl::object::Interface::Object {
	public:
		HttpObject(Engine& aEngine)
		: engine(aEngine) { }

		//void addSetting(const std::string& key, const std::string& value) override { }
		Engine& getEngine() const { return engine; }

	private:
		Engine& engine;
	};

	HttpObject httpObject;

	struct HttpServer {
		HttpServer(const std::string& id, std::uint16_t port, bool isHttps, const std::vector<std::pair<std::string, std::string>>& settings, const std::string& implementation);

		std::string id;
		std::uint16_t port;
		bool isHttps;
		std::string implementation;
		esl::object::ValueSettings settings;

		std::unique_ptr<esl::http::server::Socket> socket;

		// std::set<std::string> hostnames;
	};

	struct HttpListeners {
		std::map<std::string, std::unique_ptr<http::Listener>> listenerByHostname;
	};

	std::map<std::string, HttpServer> httpServerById;

	std::map<std::uint16_t, HttpListeners> httpListenersByPort;

	/* ***************** *
	 * Messaging methods *
	 * ***************** */

	static std::unique_ptr<esl::utility::Consumer> createMessageHandler(esl::messaging::MessageContext& messageContext);

	/* ************ *
	 * HTTP methods *
	 * ************ */

	static std::unique_ptr<esl::http::server::requesthandler::Interface::RequestHandler> createRequestHandler(esl::http::server::RequestContext& requestContext);
	http::Listener* getHttpListener(esl::http::server::RequestContext& requestContext) const;
};

} /* namespace engine */
} /* namespace jerry */

#endif /* JERRY_ENGINE_ENGINE_H_ */
