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

#include <jerry/builtin/procedure/detach/Procedure.h>
#include <jerry/engine/ObjectContext.h>
#include <jerry/engine/ProcessRegistry.h>
#include <jerry/ExceptionHandler.h>
#include <jerry/Logger.h>

#include <esl/Stacktrace.h>

#include <stdexcept>

namespace jerry {
namespace builtin {
namespace procedure {
namespace detach {

namespace {
Logger logger("jerry::builtin::procedure::detach::Procedure");
} /* anonymous namespace */

std::unique_ptr<esl::processing::procedure::Interface::Procedure> Procedure::create(const std::vector<std::pair<std::string, std::string>>& settings) {
	return std::unique_ptr<esl::processing::procedure::Interface::Procedure>(new Procedure(settings));
}

Procedure::Procedure(const std::vector<std::pair<std::string, std::string>>& settings) {
	bool hasProcedureId = false;

	for(const auto& setting : settings) {
		if(setting.first == "procedure-id") {
			if(hasProcedureId) {
				throw std::runtime_error("Multiple definition of attribute 'procedure-id'");
			}
			procedureId = setting.second;
			hasProcedureId = true;
			if(procedureId.empty()) {
				throw std::runtime_error("Invalid value \"\" for attribute 'procedure-id'");
			}
		}
		else {
			throw std::runtime_error("Unknown parameter key=\"" + setting.first + "\" with value=\"" + setting.second + "\"");
		}
	}

	if(procedureId.empty()) {
		throw std::runtime_error("Missing attribute 'procedure-id'");
	}
}

Procedure::~Procedure() {
	std::unique_lock<std::mutex> runningProceduresLock(runningProceduresMutex);
	runningProceduresCondVar.wait(runningProceduresLock, [this] {
		return runningProcedures == 0;
	});
}

void Procedure::procedureRun(esl::object::ObjectContext&) {
	std::lock_guard<std::mutex> runningProceduresLock(runningProceduresMutex);
	if(runningProceduresCancel) {
		return;
	}

	std::thread runThread([this] {
		try {
			engine::ObjectContext objectContext(processRegistry);
			if(procedure) {
				procedure->procedureRun(objectContext);
			}
		}
		catch(...) {
			ExceptionHandler exceptionHandler(std::current_exception());
	    	exceptionHandler.dump(logger.error);
		}

		{
			std::lock_guard<std::mutex> runningProceduresLock(runningProceduresMutex);
			--runningProcedures;
		}
		if(processRegistry) {
			processRegistry->processUnregister(*this);
		}
		runningProceduresCondVar.notify_one();
	});

	runThread.detach();
	++runningProcedures;
	if(processRegistry) {
		processRegistry->processRegister(*this);
	}
}

void Procedure::procedureCancel() {
	/* cancel procedure */
	std::lock_guard<std::mutex> runningProceduresLock(runningProceduresMutex);
	if(procedure && runningProcedures > 0) {
		procedure->procedureCancel();
	}
}

void Procedure::initializeContext(esl::object::ObjectContext& objectContext) {
	procedure = objectContext.findObject<esl::processing::procedure::Interface::Procedure>(procedureId);
	if(procedure == nullptr) {
		throw std::runtime_error("Cannot find procedure with id \"" + procedureId + "\"");
	}

	engine::ObjectContext* engineObjectContext = dynamic_cast<engine::ObjectContext*>(&objectContext);
	if(engineObjectContext == nullptr) {
		throw esl::addStacktrace(std::runtime_error("Engine error"));
	}
	processRegistry = engineObjectContext->getProcessRegistry();
}

} /* namespace detach */
} /* namespace procedure */
} /* namespace builtin */
} /* namespace jerry */
