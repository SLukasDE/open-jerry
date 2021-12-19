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

#include <jerry/engine/http/server/Endpoint.h>
#include <jerry/Logger.h>

#include <esl/utility/String.h>

namespace jerry {
namespace engine {
namespace http {
namespace server {

namespace {
Logger logger("jerry::engine::http::server::Endpoint");
} /* anonymous namespace */

Endpoint::Endpoint(const std::string& aPath)
: path("/" + esl::utility::String::trim(std::move(aPath), '/'))
{ }

void Endpoint::dumpTree(std::size_t depth) const {
	for(std::size_t i=0; i<depth; ++i) {
		logger.info << "|   ";
	}
	logger.info << "Path: \"" << getPath() << "\"\n";
	Context::dumpTree(depth);
}

const std::string& Endpoint::getPath() const {
	return path;
}

} /* namespace server */
} /* namespace http */
} /* namespace engine */
} /* namespace jerry */
