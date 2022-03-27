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

#include <jerry/builtin/procedure/returncode/Procedure.h>
#include <jerry/Logger.h>

#include <esl/object/Value.h>

#include <stdexcept>

namespace jerry {
namespace builtin {
namespace procedure {
namespace returncode {

namespace {
using ReturnCodeObject = esl::object::Value<int>;
Logger logger("jerry::builtin::procedure::returncode::Procedure");
} /* anonymous namespace */

std::unique_ptr<esl::processing::procedure::Interface::Procedure> Procedure::create(const std::vector<std::pair<std::string, std::string>>& settings) {
	return std::unique_ptr<esl::processing::procedure::Interface::Procedure>(new Procedure(settings));
}

Procedure::Procedure(const std::vector<std::pair<std::string, std::string>>& settings) {
	bool hasReturnCode = false;
	for(const auto& setting : settings) {
		if(setting.first == "return-code") {
			if(hasReturnCode) {
				throw std::runtime_error("Multiple definition of attribute 'return-code'");
			}
			hasReturnCode = true;
			try {
				returnCode = std::stoi(setting.second);
			}
			catch(const std::exception& e) {
				throw std::runtime_error("Value \"" + setting.second + "\" of parameter 'return-code' is invalid: " + e.what());
			}
			catch(...) {
				throw std::runtime_error("Value \"" + setting.second + "\" of parameter 'sleep-ms' is invalid.");
			}
		}
		else {
			throw std::runtime_error("Unknown parameter key=\"" + setting.first + "\" with value=\"" + setting.second + "\"");
		}
	}

	if(!hasReturnCode) {
		throw std::runtime_error("Parameter 'return-code' is missing");
	}
}

void Procedure::procedureRun(esl::object::ObjectContext& objectContext) {
	ReturnCodeObject* returnCodeObject = objectContext.findObject<ReturnCodeObject>("return-code");
	if(!returnCodeObject) {
	    objectContext.addObject("return-code", std::unique_ptr<esl::object::Interface::Object>(new ReturnCodeObject(returnCode)));
	}
	else if(returnCode != 0) {
		*returnCodeObject = returnCode;
	}
}

} /* namespace returncode */
} /* namespace procedure */
} /* namespace builtin */
} /* namespace jerry */
