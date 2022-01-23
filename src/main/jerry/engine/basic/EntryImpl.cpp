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

#include <jerry/engine/basic/EntryImpl.h>
#include <jerry/engine/basic/Context.h>
#include <jerry/Logger.h>

#include <esl/object/InitializeContext.h>

namespace jerry {
namespace engine {
namespace basic {


namespace {
Logger logger("jerry::engine::basic::EntryImpl");
} /* anonymous namespace */

EntryImpl::EntryImpl(Applications& aRefApplications)
: refApplications(&aRefApplications)
{ }

EntryImpl::EntryImpl(std::unique_ptr<esl::processing::procedure::Interface::Procedure> aProcedure)
: procedure(std::move(aProcedure))
{ }

EntryImpl::EntryImpl(esl::processing::procedure::Interface::Procedure& aRefProcedure)
: refProcedure(&aRefProcedure)
{ }

EntryImpl::EntryImpl(std::unique_ptr<Context> aContext)
: context(std::move(aContext))
{ }

EntryImpl::EntryImpl(Context& aRefContext)
: refContext(&aRefContext)
{ }

EntryImpl::EntryImpl(std::unique_ptr<esl::com::basic::server::requesthandler::Interface::RequestHandler> aRequestHandler)
: requestHandler(std::move(aRequestHandler))
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

	if(requestHandler) {
		/* ************************* *
		 * initialize requestHandler *
		 * ************************* */
		esl::object::InitializeContext* initializeContext = dynamic_cast<esl::object::InitializeContext*>(requestHandler.get());
		if(initializeContext) {
			initializeContext->initializeContext(ownerContext);
		}
	}
}

std::set<std::string> EntryImpl::getNotifiers() const {
	if(refApplications) {
		/* ****************************** *
		 * handle referenced applications *
		 * ****************************** */
		std::set<std::string> notifiers;

		for(auto& appsEntry : refApplications->getApplications()) {
			if(!appsEntry.second) {
				logger.warn << "Application \"" << appsEntry.first << "\" is null\n";
				continue;
			}

			jerry::engine::basic::Context* context = appsEntry.second->getBasicListener();
			if(context) {
				std::set<std::string> tmpNotifiers = context->getNotifiers();
				notifiers.insert(tmpNotifiers.begin(), tmpNotifiers.end());
			}
		}

		return notifiers;
	}
	if(context) {
		return context->getNotifiers();
	}
	else if(refContext) {
		return refContext->getNotifiers();
	}
	else if(requestHandler) {
		return requestHandler->getNotifiers();
	}
	return std::set<std::string>();
}

void EntryImpl::dumpTree(std::size_t depth) const {
	if(refApplications) {
		/* **************************** *
		 * dump referenced applications *
		 * **************************** */
		for(std::size_t i=0; i<depth; ++i) {
			logger.info << "|   ";
		}
		logger.info << "+-> Applications: -> " << refApplications << " (reference)\n";
	}

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
		logger.info << "+-> Context:\n";
		context->dumpTree(depth + 1);
	}

	if(refContext) {
		/* *********************** *
		 * dump referenced context *
		 * *********************** */
		for(std::size_t i=0; i<depth; ++i) {
			logger.info << "|   ";
		}
		logger.info << "+-> Context: -> " << refContext << " (reference)\n";
	}

	if(requestHandler) {
		/* ******************* *
		 * dump RequestHandler *
		 * ******************* */
		for(std::size_t i=0; i<depth; ++i) {
			logger.info << "|   ";
		}
		logger.info << "+-> BasicHandler\n";
	}
}

esl::io::Input EntryImpl::accept(esl::com::basic::server::RequestContext& requestContext) {
	if(refApplications) {
		/* ****************************** *
		 * handle referenced applications *
		 * ****************************** */

		for(auto& appsEntry : refApplications->getApplications()) {
			if(!appsEntry.second) {
				logger.warn << "Application \"" << appsEntry.first << "\" is null\n";
				continue;
			}

			jerry::engine::basic::Context* context = appsEntry.second->getBasicListener();
			if(context) {
				esl::io::Input input = context->accept(requestContext);
				if(input) {
					return input;
				}
			}
		}
	}

	if(procedure) {
		/* **************** *
		 * handle procedure *
		 * **************** */
		procedure->procedureRun(requestContext.getObjectContext());
	}

	if(refProcedure) {
		/* *************************** *
		 * handle referenced procedure *
		 * *************************** */
		refProcedure->procedureRun(requestContext.getObjectContext());
	}

	if(context) {
		/* ************** *
		 * handle context *
		 * ************** */
		return context->accept(requestContext);
	}

	if(refContext) {
		/* ************************* *
		 * handle referenced context *
		 * ************************* */
		return refContext->accept(requestContext);
	}

	if(requestHandler) {
		/* **************************** *
		 * handle requestHandlerFactory *
		 * **************************** */
		return requestHandler->accept(requestContext);
	}

	return esl::io::Input();
}


} /* namespace basic */
} /* namespace engine */
} /* namespace jerry */
