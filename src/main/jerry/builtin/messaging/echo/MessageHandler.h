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

#ifndef JERRY_BUILTIN_MESSAGING_ECHO_MESSAGEHANDLER_H_
#define JERRY_BUILTIN_MESSAGING_ECHO_MESSAGEHANDLER_H_

#include <jerry/builtin/messaging/echo/Settings.h>

#include <esl/messaging/MessageContext.h>
#include <esl/utility/Consumer.h>
#include <esl/utility/Reader.h>

#include <string>

namespace jerry {
namespace builtin {
namespace messaging {
namespace echo {

class MessageHandler : public esl::utility::Consumer {
public:
	static std::unique_ptr<esl::utility::Consumer> create(esl::messaging::MessageContext& messageContext);

	MessageHandler(esl::messaging::MessageContext& messageContext, Settings& settings);

	/* return: Reader::npos
	 *           if there is no more data to read or Consumer is not interested for reading more data
	 *
	 *         Number of characters read from Reader */
	std::size_t read(esl::utility::Reader& reader) override;

private:
	esl::messaging::MessageContext& messageContext;
	Settings& settings;
	std::string message;
};

} /* namespace echo */
} /* namespace messaging */
} /* namespace builtin */
} /* namespace jerry */

#endif /* JERRY_BUILTIN_MESSAGING_ECHO_MESSAGEHANDLER_H_ */
