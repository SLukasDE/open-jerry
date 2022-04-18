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

#ifndef JERRY_LOGGER_H_
#define JERRY_LOGGER_H_

#include <esl/logging/Logger.h>
#include <esl/logging/Level.h>
#include <esl/logging/layout/Interface.h>
#include <esl/logging/appender/Interface.h>

#include <memory>
#include <string>

namespace jerry {
class Logger : public esl::logging::Logger<esl::logging::Level::TRACE> {
public:
	using esl::logging::Logger<esl::logging::Level::TRACE>::Logger;
	//Logger(const char* aTypeName = "")

	static void flush();
	static void addLayout(const std::string& id, std::unique_ptr<esl::logging::layout::Interface::Layout> layout);
	static void addAppender(const std::string& name, const std::string& layoutRefId, std::unique_ptr<esl::logging::appender::Interface::Appender> appender);
};
} /* namespace jerry */

#endif /* JERRY_LOGGER_H_ */

