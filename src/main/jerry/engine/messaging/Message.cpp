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

#include <jerry/engine/messaging/Message.h>
#include <jerry/Logger.h>

namespace jerry {
namespace engine {
namespace messaging {

namespace {
Logger logger("jerry::engine::messaging::Message");
}

Message::Message(esl::messaging::Message& aBaseMessage)
: esl::messaging::Message(aBaseMessage.getId()),
  baseMessage(aBaseMessage),
  readerBuffered(esl::utility::ReaderBuffered(baseMessage.getReader()))
{ }

std::string Message::getValue(const std::string& key) const {
	return baseMessage.getValue(key);
}

esl::utility::ReaderBuffered& Message::getReaderBuffered() {
	return readerBuffered;
}

esl::utility::Reader& Message::getReader() {
	return getReaderBuffered();
}

} /* namespace messaging */
} /* namespace engine */
} /* namespace jerry */
