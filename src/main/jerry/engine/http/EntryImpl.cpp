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

#include <jerry/engine/http/EntryImpl.h>
#include <jerry/Logger.h>

#include <esl/object/InitializeContext.h>

namespace jerry {
namespace engine {
namespace http {


namespace {
Logger logger("jerry::engine::http::EntryImpl");
} /* anonymous namespace */

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

EntryImpl::EntryImpl(std::unique_ptr<Endpoint> aEndpoint)
: endpoint(std::move(aEndpoint))
{ }

EntryImpl::EntryImpl(std::unique_ptr<Host> aHost)
: host(std::move(aHost))
{ }

EntryImpl::EntryImpl(std::unique_ptr<esl::com::http::server::requesthandler::Interface::RequestHandler> aRequestHandler)
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

	if(host) {
		/* *************** *
		 * initialize host *
		 * *************** */
		host->initializeContext();
	}

	if(endpoint) {
		/* ******************* *
		 * initialize endpoint *
		 * ******************* */
		endpoint->initializeContext();
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
		 * dump context *
		 * ************ */
		for(std::size_t i=0; i<depth; ++i) {
			logger.info << "|   ";
		}
		logger.info << "+-> Context: -> " << context.get() << "\n";
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

	if(host) {
		/* ********* *
		 * dump host *
		 * ********* */
		for(std::size_t i=0; i<depth; ++i) {
			logger.info << "|   ";
		}
		logger.info << "+-> Host: -> " << host.get() << "\n";
		host->dumpTree(depth + 1);
	}

	if(endpoint) {
		/* ************* *
		 * dump endpoint *
		 * ************* */
		for(std::size_t i=0; i<depth; ++i) {
			logger.info << "|   ";
		}
		logger.info << "+-> Endpoint: -> " << endpoint.get() << "\n";
		endpoint->dumpTree(depth + 1);
	}

	if(requestHandler) {
		/* ******************* *
		 * dump requestHandler *
		 * ******************* */
		for(std::size_t i=0; i<depth; ++i) {
			logger.info << "|   ";
		}
		logger.info << "+-> RequestHandler: -> " << requestHandler.get() << "\n";
	}
}

esl::io::Input EntryImpl::accept(RequestContext& requestContext) {
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
		/* *************** *
		 * handle context *
		 * *************** */
		return context->accept(requestContext);
	}

	if(refContext) {
		/* ************************* *
		 * handle referenced context *
		 * ************************* */
		return refContext->accept(requestContext);
	}

	if(host && host->isMatch(requestContext.getRequest().getHostName())) {
		/* *********** *
		 * handle host *
		 * *********** */
		return host->accept(requestContext);
	}

	if(endpoint && endpoint->isMatch(requestContext.getPath())) {
		/* *************** *
		 * handle endpoint *
		 * *************** */
		std::string path = requestContext.getPath();
		requestContext.setPath(path.substr(endpoint->getPath().size()));

		esl::io::Input input = endpoint->accept(requestContext);
		if(!input) {
			requestContext.setPath(path);
		}
		return input;
	}

	if(requestHandler) {
		/* ********************** *
		 * handle request handler *
		 * ********************** */
		return requestHandler->accept(requestContext);
	}

	return esl::io::Input();
}


} /* namespace http */
} /* namespace engine */
} /* namespace jerry */
