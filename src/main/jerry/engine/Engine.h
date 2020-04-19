/*
 * This file is part of Jerry application server.
 * Copyright (C) 2020 Sven Lukas
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
#include <jerry/engine/Listener.h>
#include <jerry/utility/URL.h>

#include <esl/utility/MessageTimer.h>
#include <esl/http/server/Socket.h>
#include <esl/http/server/requesthandler/Interface.h>
#include <esl/http/server/RequestContext.h>
//#include <esl/Object.h>
#include <esl/object/Interface.h>

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
	void stop();

	void addTLSHost(const std::string& hostname, std::vector<unsigned char> certificate, std::vector<unsigned char> key);
	void addCertificate(const std::string& domain, const std::string& keyFile, const std::string& certificateFile);

	Listener& addListener(utility::URL url);

	esl::object::Interface::Object* getObject(const std::string& id) const override;

private:
	class EngineObject : public esl::object::Interface::Object {
	public:
		EngineObject(Engine& aEngine)
		: engine(aEngine) { }

		//void addSetting(const std::string& key, const std::string& value) override { }
		Engine& getEngine() const { return engine; }

	private:
		Engine& engine;
	};

	struct PortListener {
		PortListener(std::string aProtocol)
		: protocol(aProtocol)
		{ }
		std::string protocol;
		std::map<std::string, std::unique_ptr<Listener>> listenerByDomain;
	};

	EngineObject engineObject;

    std::thread::id runThreadId = std::thread::id();

    esl::utility::MessageTimer<std::string, Message> messageTimer;
	std::map<std::string, std::pair<std::vector<unsigned char>, std::vector<unsigned char>>> certsByHostname;

	std::map<uint16_t, PortListener> listenerByPort;

	static std::unique_ptr<esl::http::server::requesthandler::Interface::RequestHandler> createRequestHandler(esl::http::server::RequestContext& requestContext);
	Listener* getListener(esl::http::server::RequestContext& requestContext) const;
};

} /* namespace engine */
} /* namespace jerry */

#endif /* JERRY_ENGINE_ENGINE_H_ */
