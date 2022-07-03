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

#include <jerry/engine/procedure/Context.h>
#include <jerry/engine/procedure/EntryImpl.h>
//#include <jerry/ExceptionHandler.h>
#include <jerry/Logger.h>


#include <stdexcept>

namespace jerry {
namespace engine {
namespace procedure {

namespace {
Logger logger("jerry::engine::procedure::Context");
} /* anonymous namespace */

void Context::addProcedure(std::unique_ptr<esl::processing::Procedure> procedure) {
	entries.emplace_back(new EntryImpl(std::move(procedure)));
}

void Context::addProcedure(const std::string& refId) {
	esl::processing::Procedure* procedure = findObject<esl::processing::Procedure>(refId);

	if(procedure == nullptr) {
	    throw std::runtime_error("No procedure found with ref-id=\"" + refId + "\".");
	}

	entries.emplace_back(new EntryImpl(*procedure));
}

void Context::addContext(std::unique_ptr<Context> context) {
	entries.emplace_back(new EntryImpl(std::move(context)));
}

void Context::addContext(const std::string& refId) {
	Context* context = findObject<Context>(refId);

	if(context == nullptr) {
	    throw std::runtime_error("No context found with ref-id=\"" + refId + "\".");
	}

	entries.emplace_back(new EntryImpl(*context));
}

void Context::initializeContext() {
	ObjectContext::initializeContext();

	// call initializeContext() of sub-context's
	for(auto& entry : entries) {
		entry->initializeContext(*this);
	}
}

void Context::dumpTree(std::size_t depth) const {
	for(std::size_t i=0; i<depth; ++i) {
		logger.info << "|   ";
	}
	logger.info << "+-> Context\n";
	++depth;

	ObjectContext::dumpTree(depth);

	for(auto& entry : entries) {
		entry->dumpTree(depth);
	}
}

void Context::procedureRun(esl::object::Context& objectContext) {
	for(auto& entry : entries) {
		{
			std::lock_guard<std::mutex> runningProceduresLock(runningProceduresMutex);
			if(runningProceduresCancel) {
				break;
			}
			++runningProcedures[entry.get()];
		}

		entry->procedureRun(objectContext);

		{
			std::lock_guard<std::mutex> runningProceduresLock(runningProceduresMutex);
			auto iter = runningProcedures.find(entry.get());
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

void Context::procedureCancel() {
	std::lock_guard<std::mutex> runningProceduresLock(runningProceduresMutex);

	runningProceduresCancel = !runningProcedures.empty();
	for(auto& runningProcedure : runningProcedures) {
		runningProcedure.first->procedureCancel();
	}
}

void Context::setProcessRegistry(ProcessRegistry* processRegistry) {
	ObjectContext::setProcessRegistry(processRegistry);
	for(auto& entry : entries) {
		entry->setProcessRegistry(processRegistry);
	}
}

} /* namespace procedure */
} /* namespace engine */
} /* namespace jerry */
