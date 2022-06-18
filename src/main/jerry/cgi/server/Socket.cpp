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

#include <jerry/cgi/server/Socket.h>

#include <cstdlib>
#include <string>
#include <utility>
#include <vector>

namespace jerry {
namespace cgi {


namespace {
std::string getEnvVar(std::string const & key) {
	char* value = std::getenv( key.c_str() );
	return value ? value : "";
}
}


std::unique_ptr<esl::com::http::server::Interface::Socket> Socket::create(const std::vector<std::pair<std::string, std::string>>& settings) {
	return std::unique_ptr<esl::com::http::server::Interface::Socket>(new Socket(settings));
}

Socket::Socket(const std::vector<std::pair<std::string, std::string>>& settings)
{ }

void Socket::addTLSHost(const std::string& hostname, std::vector<unsigned char> certificate, std::vector<unsigned char> key) {
}

void Socket::listen(const esl::com::http::server::requesthandler::Interface::RequestHandler& requestHandler, std::function<void()> onReleasedHandler) {
}

void Socket::release() {
}


} /* namespace cgi */
} /* namespace jerry */
