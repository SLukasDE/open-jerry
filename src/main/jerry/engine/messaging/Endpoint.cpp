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

#include <jerry/engine/messaging/Endpoint.h>
#include <jerry/Logger.h>

#include <esl/Stacktrace.h>

#include <stdexcept>

namespace jerry {
namespace engine {
namespace messaging {

namespace {
Logger logger("jerry::engine::messaging::Endpoint");
} /* anonymous namespace */

Endpoint::Endpoint(Listener& listener, const Context& parentContext, std::string aQueueName, bool inheritObjects)
: Context(listener, this, &parentContext, inheritObjects),
  queueName(std::move(aQueueName))
{ }

// only used by Listener
Endpoint::Endpoint(Listener& listener, bool inheritObjects)
: Context(listener, nullptr, nullptr, inheritObjects)
{ }

const std::string& Endpoint::getQueueName() const {
	return queueName;
}

void Endpoint::dumpTree(std::size_t depth) const {
	for(std::size_t i=0; i<depth; ++i) {
		logger.info << "|   ";
	}
	logger.info << "Queue: \"" << getQueueName() << "\"\n";

	Context::dumpTree(depth);
}

} /* namespace messaging */
} /* namespace engine */
} /* namespace jerry */
