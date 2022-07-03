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

#ifndef JERRY_CGI_SERVER_SOCKET_H_
#define JERRY_CGI_SERVER_SOCKET_H_

#include <esl/com/http/server/RequestHandler.h>
#include <esl/com/http/server/Socket.h>
#include <esl/com/http/server/Request.h>
#include <esl/object/Object.h>

#include <cstdint>
#include <map>

namespace jerry {
namespace cgi {


class Socket : public esl::com::http::server::Socket {
public:
	static inline const char* getImplementation() {
		return "cgi4esl";
	}

	static std::unique_ptr<esl::com::http::server::Socket> create(const std::vector<std::pair<std::string, std::string>>& settings);

	Socket(const std::vector<std::pair<std::string, std::string>>& settings);

	void addTLSHost(const std::string& hostname, std::vector<unsigned char> certificate, std::vector<unsigned char> key) override;

	void listen(const esl::com::http::server::RequestHandler& requestHandler, std::function<void()> onReleasedHandler) override;
	void release() override;
	//bool wait(std::uint32_t ms) override;

private:
};


} /* namespace cgi */
} /* namespace jerry */

#endif /* JERRY_CGI_SERVER_SOCKET_H_ */
