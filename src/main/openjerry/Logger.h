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

#ifndef OPENJERRY_LOGGER_H_
#define OPENJERRY_LOGGER_H_

#include <esl/monitoring/Streams.h>
#include <esl/monitoring/Logger.h>

namespace openjerry {

#ifdef OPENJERRY_LOGGING_LEVEL_DEBUG
using Logger = esl::monitoring::Logger<esl::monitoring::Streams::Level::trace>;
#else
using Logger = esl::monitoring::Logger<esl::monitoring::Streams::Level::trace>;
//using Logger = esl::monitoring::Logger<esl::monitoring::Streams::Level::error>;
#endif

} /* namespace openjerry */

#endif /* OPENJERRY_LOGGER_H_ */

