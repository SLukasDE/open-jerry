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

#include <jerry/builtin/procedure/sleep/Procedure.h>
#include <jerry/Logger.h>

#include <thread>
#include <string>
#include <stdexcept>

namespace jerry {
namespace builtin {
namespace procedure {
namespace sleep {

namespace {
Logger logger("jerry::builtin::procedure::sleep::Procedure");
} /* anonymous namespace */

std::unique_ptr<esl::processing::procedure::Interface::Procedure> Procedure::createProcedure(const esl::module::Interface::Settings& settings) {
	return std::unique_ptr<esl::processing::procedure::Interface::Procedure>(new Procedure(settings));
}

Procedure::Procedure(const esl::module::Interface::Settings& settings) {
	for(const auto& setting : settings) {
		if(setting.first == "sleep-ms") {
			if(setting.second.empty()) {
				throw std::runtime_error("Value \"\" of parameter 'sleep-ms' is invalid");

			}
			sleepMs = std::chrono::milliseconds(std::stoul(setting.second));
			if(sleepMs == std::chrono::milliseconds(0)) {
				throw std::runtime_error("Value \"0\" of parameter 'sleep-ms' is invalid");
			}
		}
		else {
			throw std::runtime_error("Unknown parameter key=\"" + setting.first + "\" with value=\"" + setting.second + "\"");
		}

	}
	if(sleepMs == std::chrono::milliseconds(0)) {
		throw std::runtime_error("Parameter 'sleep-ms' is missing");
	}
}

void Procedure::procedureRun(esl::object::ObjectContext& objectContext) {
	logger.debug << "before sleep\n";
	std::this_thread::sleep_for(std::chrono::milliseconds(sleepMs));
	logger.debug << "after sleep\n";
}

} /* namespace sleep */
} /* namespace procedure */
} /* namespace builtin */
} /* namespace jerry */
