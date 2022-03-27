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

#include <jerry/builtin/procedure/list/Procedure.h>
#include <jerry/Logger.h>

#include <stdexcept>

namespace jerry {
namespace builtin {
namespace procedure {
namespace list {

namespace {
Logger logger("jerry::builtin::procedure::list::Procedure");
} /* anonymous namespace */

std::unique_ptr<esl::processing::procedure::Interface::Procedure> Procedure::create(const std::vector<std::pair<std::string, std::string>>& settings) {
	return std::unique_ptr<esl::processing::procedure::Interface::Procedure>(new Procedure(settings));
}

Procedure::Procedure(const std::vector<std::pair<std::string, std::string>>& settings) {
	for(const auto& setting : settings) {
		if(setting.first == "procedure-id") {
			if(setting.second.empty()) {
				throw std::runtime_error("Value \"\" of parameter 'procedure-id' is invalid");

			}
			logger.debug << "adding procedure-id '" << setting.second << "'\n";
			procedureIds.push_back(setting.second);
		}
		else {
			throw std::runtime_error("Unknown parameter key=\"" + setting.first + "\" with value=\"" + setting.second + "\"");
		}
	}
}

void Procedure::initializeContext(esl::object::ObjectContext& objectContext) {
	logger.debug << "Initialize: Lookup " << procedureIds.size() << " procedure-ids\n";
	for(const auto& procedureId : procedureIds) {
		esl::processing::procedure::Interface::Procedure* procedure = objectContext.findObject<esl::processing::procedure::Interface::Procedure>(procedureId);
		if(procedure == nullptr) {
			throw std::runtime_error("Cannot find procedure with id \"" + procedureId + "\"");
		}
		procedures.push_back(procedure);
	}
	logger.debug << procedures.size() << " procedures\n";
}

void Procedure::procedureRun(esl::object::ObjectContext& objectContext) {
	for(auto procedure : procedures) {
		{
			std::lock_guard<std::mutex> runningProceduresLock(runningProceduresMutex);
			if(runningProceduresCancel) {
				break;
			}
			++runningProcedures[procedure];
		}

		procedure->procedureRun(objectContext);

		{
			std::lock_guard<std::mutex> runningProceduresLock(runningProceduresMutex);
			auto iter = runningProcedures.find(procedure);
			if(iter != runningProcedures.end()) {
				--iter->second;
				if(iter->second == 0) {
					runningProcedures.erase(iter);
				}
			}
		}
	}

	std::lock_guard<std::mutex> runningProceduresLock(runningProceduresMutex);
	if(runningProcedures.empty()){
		runningProceduresCancel = false;
	}
}

void Procedure::procedureCancel() {
	std::lock_guard<std::mutex> runningProceduresLock(runningProceduresMutex);

	runningProceduresCancel = !runningProcedures.empty();
	for(auto& runningProcedure : runningProcedures) {
		runningProcedure.first->procedureCancel();
	}
}

} /* namespace list */
} /* namespace procedure */
} /* namespace builtin */
} /* namespace jerry */
