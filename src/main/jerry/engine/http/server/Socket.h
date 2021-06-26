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

#ifndef JERRY_ENGINE_HTTP_SERVER_SOCKET_H_
#define JERRY_ENGINE_HTTP_SERVER_SOCKET_H_

#include <jerry/engine/http/server/Listener.h>

#include <esl/com/http/server/Interface.h>
#include <esl/com/http/server/Socket.h>
#include <esl/com/http/server/requesthandler/Interface.h>
#include <esl/object/Interface.h>

#include <cstdint>
#include <vector>
#include <map>
#include <set>
#include <utility>
#include <string>


namespace jerry {
namespace engine {
class Engine;
} /* namespace engine */
} /* namespace jerry */

namespace jerry {
namespace engine {
namespace http {
namespace server {

class Socket final : public esl::com::http::server::Interface::Socket {
public:
	Socket(Engine& engine, const std::string& id, bool https,
			const esl::object::Interface::Settings& settings, const std::string& implementation);

	void addTLSHost(const std::string& hostname, std::vector<unsigned char> certificate, std::vector<unsigned char> key) override;
	void addObjectFactory(const std::string& id, ObjectFactory objectFactory) override;
	void listen(esl::com::http::server::requesthandler::Interface::CreateInput createInput) override;
	void release() override;
	bool wait(std::uint32_t ms) override;

	void dumpTree(std::size_t depth) const;

	void addListener(Listener& listener);
/*
	void Socket::engineListen() {
		getSocket().listen(createRequestHandler);
	}
*/
	esl::com::http::server::Interface::Socket& getSocket() noexcept;
	const std::string& getId() const noexcept;
	const std::string& getImplementation() const noexcept;
	bool isHttps() const noexcept;

	std::set<std::string> getHostnames() const;

	static esl::io::Input createRequestHandler(esl::com::http::server::RequestContext& baseRequestContext);

private:
	Listener* getListenerByHostname(const std::string& hostname);

	esl::com::http::server::Socket socket;

	Engine& engine;
	const std::string id;
	const bool https;
	const std::string implementation;
	const esl::object::Interface::Settings settings;

	//std::map<std::string, std::unique_ptr<Listener>> listenerByHostname;
	std::map<std::string, Listener*> listenerByHostname;
	std::vector<std::reference_wrapper<Listener>> refListeners;
};

} /* namespace server */
} /* namespace http */
} /* namespace engine */
} /* namespace jerry */

#endif /* JERRY_ENGINE_HTTP_SERVER_SOCKET_H_ */
