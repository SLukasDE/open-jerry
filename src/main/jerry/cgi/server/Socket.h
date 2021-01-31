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

#ifndef JERRY_CGI_SERVER_SOCKET_H_
#define JERRY_CGI_SERVER_SOCKET_H_

#include <esl/http/server/requesthandler/Interface.h>
#include <esl/http/server/Interface.h>
#include <esl/http/server/Request.h>
#include <esl/object/Values.h>

#include <cstdint>
#include <string>
#include <map>

namespace jerry {
namespace cgi {
namespace server {

class Socket : public esl::http::server::Interface::Socket {
public:
	static inline const char* getImplementation() {
		return "cgi4esl";
	}

	static std::unique_ptr<esl::http::server::Interface::Socket> create(uint16_t port, esl::http::server::requesthandler::Interface::CreateRequestHandler createRequestHandler, const esl::object::Values<std::string>& values);

	Socket(uint16_t port, esl::http::server::requesthandler::Interface::CreateRequestHandler createRequestHandler, const esl::object::Values<std::string>& values);

	void addTLSHost(const std::string& hostname, std::vector<unsigned char> certificate, std::vector<unsigned char> key) override;

	void addObjectFactory(const std::string& id, ObjectFactory objectFactory) override;

	bool listen() override;
	void release() override;

private:
	uint16_t port;
	std::map<std::string, ObjectFactory> objectFactories;
};

} /* namespace server */
} /* namespace cgi */
} /* namespace jerry */

#endif /* JERRY_CGI_SERVER_SOCKET_H_ */
