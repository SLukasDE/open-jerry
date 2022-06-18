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

#ifndef JERRY_EXCEPTIONMESSAGE_H_
#define JERRY_EXCEPTIONMESSAGE_H_

#include <esl/stacktrace/Stacktrace.h>

#include <string>
#include <memory>

namespace jerry {

struct ExceptionMessage final {
	std::string title;
	std::string message;

	std::unique_ptr<esl::stacktrace::Stacktrace> stacktrace;
};

} /* namespace jerry */

#endif /* JERRY_EXCEPTIONMESSAGE_H_ */
