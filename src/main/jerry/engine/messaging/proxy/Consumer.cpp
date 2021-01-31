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

#include <jerry/engine/messaging/proxy/Consumer.h>
#include <jerry/Logger.h>

#include <esl/Stacktrace.h>

#include <stdexcept>

namespace jerry {
namespace engine {
namespace messaging {
namespace proxy {

namespace {
Logger logger("jerry::engine::messaging::proxy::Consumer");
} /* anonymous namespace */

void Consumer::start(const std::set<std::string>& queues, esl::messaging::messagehandler::Interface::CreateMessageHandler createMessageHandler, std::uint16_t numThreads, bool stopIfEmpty) {
	throw esl::addStacktrace(std::runtime_error("Starting consumer is not allowed."));
}

void Consumer::stop() {
	throw esl::addStacktrace(std::runtime_error("Stopping consumer is not allowed."));
}

bool Consumer::wait(std::uint32_t ms) {
	throw esl::addStacktrace(std::runtime_error("Waiting for stopped consumer is not allowed."));
}

} /* namespace proxy */
} /* namespace messaging */
} /* namespace engine */
} /* namespace jerry */
