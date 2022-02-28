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

std::unique_ptr<esl::processing::procedure::Interface::Procedure> Procedure::createProcedure(const esl::module::Interface::Settings& settings) {
	return std::unique_ptr<esl::processing::procedure::Interface::Procedure>(new Procedure(settings));
}

Procedure::Procedure(const esl::module::Interface::Settings& settings) {
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
	std::lock_guard<std::mutex> runningLock(runningMutex);

	{
		std::lock_guard<std::mutex> currentProcedureLock(currentProcedureMutex);
		currentProcedureCancel = false;
	}

	for(auto procedure : procedures) {
		{
			std::lock_guard<std::mutex> currentProcedureLock(currentProcedureMutex);
			if(currentProcedureCancel) {
				break;
			}

			currentProcedure = procedure;
		}
		procedure->procedureRun(objectContext);
	}

	{
		std::lock_guard<std::mutex> currentProcedureLock(currentProcedureMutex);
		currentProcedure = nullptr;
	}
}

void Procedure::procedureCancel() {
	std::lock_guard<std::mutex> currentProcedureLock(currentProcedureMutex);
	currentProcedureCancel = true;
	if(currentProcedure) {
		currentProcedure->procedureCancel();
	}
}

} /* namespace list */
} /* namespace procedure */
} /* namespace builtin */
} /* namespace jerry */
