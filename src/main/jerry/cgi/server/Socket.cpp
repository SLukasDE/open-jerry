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

#include <jerry/cgi/server/Socket.h>

#include <cstdlib>

namespace jerry {
namespace cgi {
namespace server {

namespace {
std::string getEnvVar(std::string const & key) {
	char* value = std::getenv( key.c_str() );
	return value ? value : "";
}
}


std::unique_ptr<esl::http::server::Interface::Socket> Socket::create(uint16_t port, esl::http::server::requesthandler::Interface::CreateRequestHandler createRequestHandler, const esl::object::Values<std::string>& values) {
	return std::unique_ptr<esl::http::server::Interface::Socket>(new Socket(port, createRequestHandler, values));
}

Socket::Socket(uint16_t aPort, esl::http::server::requesthandler::Interface::CreateRequestHandler createRequestHandler, const esl::object::Values<std::string>& values)
: esl::http::server::Interface::Socket(),
  port(aPort)
{

}

void Socket::addTLSHost(const std::string& hostname, std::vector<unsigned char> certificate, std::vector<unsigned char> key) {
}

void Socket::addObjectFactory(const std::string& id, ObjectFactory objectFactory) {
	objectFactories[id] = objectFactory;
}

bool Socket::listen() {
	return true;
}

void Socket::release() {
}

} /* namespace server */
} /* namespace cgi */
} /* namespace jerry */
