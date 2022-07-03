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

#include <jerry/builtin/object/applications/EntryImpl.h>
#include <jerry/engine/main/Context.h>
#include <jerry/engine/basic/Context.h>
#include <jerry/Logger.h>

#include <esl/object/InitializeContext.h>

#include <string>

namespace jerry {
namespace builtin {
namespace object {
namespace applications {

namespace {
Logger logger("jerry::builtin::object::applications::EntryImpl");
} /* anonymous namespace */

EntryImpl::EntryImpl(std::unique_ptr<esl::processing::Procedure> aProcedure)
: procedure(std::move(aProcedure))
{ }

EntryImpl::EntryImpl(esl::processing::Procedure& aRefProcedure)
: refProcedure(&aRefProcedure)
{ }

EntryImpl::EntryImpl(std::unique_ptr<engine::procedure::Context> aProcedureContext)
: procedureContext(std::move(aProcedureContext))
{ }

EntryImpl::EntryImpl(engine::procedure::Context& aRefProcedureContext)
: refProcedureContext(&aRefProcedureContext)
{ }

EntryImpl::EntryImpl(std::unique_ptr<engine::basic::Context> aBasicContext)
: basicContext(std::move(aBasicContext))
{ }

EntryImpl::EntryImpl(engine::basic::Context& aRefBasicContext)
: refBasicContext(&aRefBasicContext)
{ }

EntryImpl::EntryImpl(std::unique_ptr<engine::http::Context> aHttpContext)
: httpContext(std::move(aHttpContext))
{ }

EntryImpl::EntryImpl(engine::http::Context& aRefHttpContext)
: refHttpContext(&aRefHttpContext)
{ }

void EntryImpl::initializeContext(Application& ownerContext) {
	if(procedure) {
		/* ******************** *
		 * initialize procedure *
		 * ******************** */
		esl::object::InitializeContext* initializeContext = dynamic_cast<esl::object::InitializeContext*>(procedure.get());
		if(initializeContext) {
			initializeContext->initializeContext(ownerContext);
		}
	}

	if(procedureContext) {
		/* ****************** *
		 * initialize context *
		 * ****************** */
		procedureContext->initializeContext();
	}

	if(basicContext) {
		/* ****************** *
		 * initialize context *
		 * ****************** */
		basicContext->initializeContext();
	}

	if(httpContext) {
		/* ****************** *
		 * initialize context *
		 * ****************** */
		httpContext->initializeContext();
	}
}

esl::io::Input EntryImpl::accept(engine::http::RequestContext& requestContext, const esl::object::Object* object) {
	if(procedure && (object == nullptr || object == procedure.get())) {
		/* *************** *
		 * start procedure *
		 * *************** */
		procedure->procedureRun(requestContext.getObjectContext());
	}

	if(refProcedure && (object == nullptr || object == refProcedure)) {
		/* ************************** *
		 * start referenced procedure *
		 * ************************** */
		refProcedure->procedureRun(requestContext.getObjectContext());
	}

	if(procedureContext && (object == nullptr || object == procedureContext.get())) {
		/* *********************** *
		 * start procedure context *
		 * *********************** */
		procedureContext->procedureRun(requestContext.getObjectContext());
	}

	if(refProcedureContext && (object == nullptr || object == refProcedureContext)) {
		/* ********************************** *
		 * start referenced procedure context *
		 * ********************************** */
		refProcedureContext->procedureRun(requestContext.getObjectContext());
	}

	if(httpContext && (object == nullptr || object == httpContext.get())) {
		/* ************** *
		 * accept context *
		 * ************** */
		return httpContext->accept(requestContext);
	}

	if(refHttpContext) {
		/* ********************************* *
		 * accept context referenced context *
		 * ********************************* */
		return refHttpContext->accept(requestContext);
	}

	return esl::io::Input();
}

esl::io::Input EntryImpl::accept(esl::com::basic::server::RequestContext& requestContext, const esl::object::Object* object) {
	if(procedure && (object == nullptr || object == procedure.get())) {
		/* *************** *
		 * start procedure *
		 * *************** */
		procedure->procedureRun(requestContext.getObjectContext());
	}

	if(refProcedure && (object == nullptr || object == refProcedure)) {
		/* ************************** *
		 * start referenced procedure *
		 * ************************** */
		refProcedure->procedureRun(requestContext.getObjectContext());
	}

	if(procedureContext && (object == nullptr || object == procedureContext.get())) {
		/* *********************** *
		 * start procedure context *
		 * *********************** */
		procedureContext->procedureRun(requestContext.getObjectContext());
	}

	if(refProcedureContext && (object == nullptr || object == refProcedureContext)) {
		/* ********************************** *
		 * start referenced procedure context *
		 * ********************************** */
		refProcedureContext->procedureRun(requestContext.getObjectContext());
	}

	if(basicContext && (object == nullptr || object == basicContext.get())) {
		/* ************** *
		 * accept context *
		 * ************** */
		return basicContext->accept(requestContext);
	}

	if(refBasicContext) {
		/* ********************************* *
		 * accept context referenced context *
		 * ********************************* */
		return refBasicContext->accept(requestContext);
	}

	return esl::io::Input();
}

void EntryImpl::procedureRun(esl::object::Context& objectContext, const esl::object::Object* object) {
	if(procedure && (object == nullptr || object == procedure.get())) {
		/* *************** *
		 * start procedure *
		 * *************** */
		procedure->procedureRun(objectContext);
	}

	if(refProcedure && (object == nullptr || object == refProcedure)) {
		/* ************************** *
		 * start referenced procedure *
		 * ************************** */
		refProcedure->procedureRun(objectContext);
	}

	if(procedureContext && (object == nullptr || object == procedureContext.get())) {
		/* *********************** *
		 * start procedure context *
		 * *********************** */
		procedureContext->procedureRun(objectContext);
	}

	if(refProcedureContext && (object == nullptr || object == refProcedureContext)) {
		/* ********************************** *
		 * start referenced procedure context *
		 * ********************************** */
		refProcedureContext->procedureRun(objectContext);
	}
}

void EntryImpl::procedureCancel() {
	if(procedure) {
		/* *************** *
		 * start procedure *
		 * *************** */
		procedure->procedureCancel();
	}

	if(refProcedure) {
		/* ************************** *
		 * start referenced procedure *
		 * ************************** */
		refProcedure->procedureCancel();
	}

	if(procedureContext) {
		/* *********************** *
		 * start procedure context *
		 * *********************** */
		procedureContext->procedureCancel();
	}

	if(refProcedureContext) {
		/* ********************************** *
		 * start referenced procedure context *
		 * ********************************** */
		refProcedureContext->procedureCancel();
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

	if(procedureContext) {
		/* ************ *
		 * dump Context *
		 * ************ */
		for(std::size_t i=0; i<depth; ++i) {
			logger.info << "|   ";
		}
		logger.info << "+-> Procedure-Context: -> " << procedureContext.get() << "\n";
		procedureContext->dumpTree(depth + 1);
	}

	if(refProcedureContext) {
		/* *********************** *
		 * dump referenced context *
		 * *********************** */
		for(std::size_t i=0; i<depth; ++i) {
			logger.info << "|   ";
		}
		logger.info << "+-> Procedure-Context: -> " << refProcedureContext << " (reference)\n";
	}

	if(basicContext) {
		/* ************ *
		 * dump Context *
		 * ************ */
		for(std::size_t i=0; i<depth; ++i) {
			logger.info << "|   ";
		}
		logger.info << "+-> Basic-Context: -> " << basicContext.get() << "\n";
		basicContext->dumpTree(depth + 1);
	}

	if(refBasicContext) {
		/* *********************** *
		 * dump referenced context *
		 * *********************** */
		for(std::size_t i=0; i<depth; ++i) {
			logger.info << "|   ";
		}
		logger.info << "+-> Basic-Context: -> " << refBasicContext << " (reference)\n";
	}

	if(httpContext) {
		/* ************ *
		 * dump Context *
		 * ************ */
		for(std::size_t i=0; i<depth; ++i) {
			logger.info << "|   ";
		}
		logger.info << "+-> Http-Context: -> " << httpContext.get() << "\n";
		httpContext->dumpTree(depth + 1);
	}

	if(refHttpContext) {
		/* *********************** *
		 * dump referenced context *
		 * *********************** */
		for(std::size_t i=0; i<depth; ++i) {
			logger.info << "|   ";
		}
		logger.info << "+-> Http-Context: -> " << refHttpContext << " (reference)\n";
	}
}

} /* namespace applications */
} /* namespace object */
} /* namespace builtin */
} /* namespace jerry */
