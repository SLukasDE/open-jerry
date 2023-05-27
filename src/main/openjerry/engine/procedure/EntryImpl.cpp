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

#include <openjerry/engine/procedure/EntryImpl.h>
#include <openjerry/engine/main/Context.h>
#include <openjerry/Logger.h>

#include <esl/object/InitializeContext.h>

namespace openjerry {
namespace engine {
namespace procedure {

namespace {
Logger logger("openjerry::engine::procedure::EntryImpl");
} /* anonymous namespace */

EntryImpl::EntryImpl(std::unique_ptr<esl::processing::Procedure> aProcedure)
: procedure(std::move(aProcedure))
{ }

EntryImpl::EntryImpl(esl::processing::Procedure& aRefProcedure)
: refProcedure(&aRefProcedure)
{ }

EntryImpl::EntryImpl(std::unique_ptr<procedure::Context> aContext)
: context(std::move(aContext))
{ }

EntryImpl::EntryImpl(procedure::Context& aRefContext)
: refContext(&aRefContext)
{ }

void EntryImpl::initializeContext(Context& ownerContext) {
	if(procedure) {
		/* ******************** *
		 * initialize procedure *
		 * ******************** */
		esl::object::InitializeContext* initializeContext = dynamic_cast<esl::object::InitializeContext*>(procedure.get());
		if(initializeContext) {
			initializeContext->initializeContext(ownerContext);
		}
	}

	if(context) {
		/* ****************** *
		 * initialize context *
		 * ****************** */
		context->initializeContext();
	}
}

void EntryImpl::dumpTree(std::size_t depth) const {
	if(procedure) {
		/* ************** *
		 * dump procedure *
		 * ************** */
		for(std::size_t i=0; i<depth; ++i) {
			logger.info << "|   ";
		}
		logger.info << "+-> Procedure: -> " << procedure.get() << "\n";
	}

	if(refProcedure) {
		/* ************************* *
		 * dump referenced procedure *
		 * ************************* */
		for(std::size_t i=0; i<depth; ++i) {
			logger.info << "|   ";
		}
		logger.info << "+-> Procedure: -> " << refProcedure << " (reference)\n";
	}

	if(context) {
		/* ************ *
		 * dump Context *
		 * ************ */
		for(std::size_t i=0; i<depth; ++i) {
			logger.info << "|   ";
		}
		logger.info << "+-> Procedure-Context:\n";
		context->dumpTree(depth + 1);
	}

	if(refContext) {
		/* *********************** *
		 * dump referenced context *
		 * *********************** */
		for(std::size_t i=0; i<depth; ++i) {
			logger.info << "|   ";
		}
		logger.info << "+-> Procedure-Context: -> " << refContext << " (reference)\n";
	}
}

void EntryImpl::procedureRun(esl::object::Context& objectContext) {
	if(procedure) {
		/* ************* *
		 * run procedure *
		 * ************* */
		logger.debug << "Start procedure ...\n";
		procedure->procedureRun(objectContext);
		logger.debug << "... procedure started.\n";
	}

	if(refProcedure) {
		/* ************************ *
		 * run referenced procedure *
		 * ************************ */
		logger.debug << "Start referenced procedure ...\n";
		refProcedure->procedureRun(objectContext);
		logger.debug << "... referenced procedure started.\n";
	}

	if(context) {
		/* ********************* *
		 * run procedure context *
		 * ********************* */
		logger.debug << "Start procedure context ...\n";
		context->procedureRun(objectContext);
		logger.debug << "... procedure context started.\n";
	}

	if(refContext) {
		/* ******************************** *
		 * run referenced procedure context *
		 * ******************************** */
		logger.debug << "Start referenced procedure context ...\n";
		refContext->procedureRun(objectContext);
		logger.debug << "... referenced procedure context started.\n";
	}
}

void EntryImpl::procedureCancel() {
	if(procedure) {
		/* **************** *
		 * cancel procedure *
		 * **************** */
		logger.debug << "Start procedure ...\n";
		procedure->procedureCancel();
		logger.debug << "... procedure started.\n";
	}

	if(refProcedure) {
		/* *************************** *
		 * cancel referenced procedure *
		 * *************************** */
		logger.debug << "Start referenced procedure ...\n";
		refProcedure->procedureCancel();
		logger.debug << "... referenced procedure started.\n";
	}

	if(context) {
		/* ************************ *
		 * cancel procedure context *
		 * ************************ */
		logger.debug << "Start procedure context ...\n";
		context->procedureCancel();
		logger.debug << "... procedure context started.\n";
	}

	if(refContext) {
		/* *********************************** *
		 * cancel referenced procedure context *
		 * *********************************** */
		logger.debug << "Start referenced procedure context ...\n";
		refContext->procedureCancel();
		logger.debug << "... referenced procedure context started.\n";
	}
}

void EntryImpl::setProcessRegistry(ProcessRegistry* processRegistry) {
	if(context) {
		context->setProcessRegistry(processRegistry);
	}
}

} /* namespace procedure */
} /* namespace engine */
} /* namespace openjerry */
