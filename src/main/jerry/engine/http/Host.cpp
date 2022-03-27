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

#include <jerry/engine/http/Host.h>
#include <jerry/Logger.h>

namespace jerry {
namespace engine {
namespace http {


namespace {
Logger logger("jerry::engine::http::Host");
} /* anonymous namespace */

Host::Host(ProcessRegistry& processRegistry, const std::string& aServerName)
: Context(processRegistry),
  serverName(aServerName)
{ }

void Host::dumpTree(std::size_t depth) const {
	for(std::size_t i=0; i<depth; ++i) {
		logger.info << "|   ";
	}
	logger.info << "Server name: \"" << getServerName() << "\"\n";
	Context::dumpTree(depth);
}

const std::string& Host::getServerName() const noexcept {
	return serverName;
}

bool Host::isMatch(const std::string& hostName) const {
	if(serverName == "*" || serverName == hostName) {
		return true;
	}

	std::string newHostName = hostName;
	for(std::string::size_type pos = newHostName.find_first_of('.'); pos != std::string::npos ; pos = newHostName.find_first_of('.')) {
		newHostName = newHostName.substr(pos+1);
		logger.debug << "Lookup for server-name \"*." << newHostName << "\"\n";
		if("*." + newHostName == serverName) {
			return true;
		}
	};

	return false;
}


} /* namespace http */
} /* namespace engine */
} /* namespace jerry */
