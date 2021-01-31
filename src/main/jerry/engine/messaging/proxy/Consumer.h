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

#ifndef JERRY_ENGINE_MESSAGING_PROXY_CONSUMER_H_
#define JERRY_ENGINE_MESSAGING_PROXY_CONSUMER_H_

#include <esl/messaging/Consumer.h>
#include <esl/messaging/messagehandler/Interface.h>

#include <cstdint>
#include <set>
#include <string>

namespace jerry {
namespace engine {
namespace messaging {
namespace proxy {

class Consumer : public esl::messaging::Consumer {
public:
	void start(const std::set<std::string>& queues, esl::messaging::messagehandler::Interface::CreateMessageHandler createMessageHandler, std::uint16_t numThreads, bool stopIfEmpty) override;
	void stop() override;
	bool wait(std::uint32_t ms) override;
};

} /* namespace proxy */
} /* namespace messaging */
} /* namespace engine */
} /* namespace jerry */

#endif /* JERRY_ENGINE_MESSAGING_PROXY_CONSUMER_H_ */
