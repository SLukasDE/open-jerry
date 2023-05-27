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

#include <openjerry/engine/http/EntryImpl.h>
#include <openjerry/Logger.h>

#include <esl/object/InitializeContext.h>

namespace openjerry {
namespace engine {
namespace http {


namespace {
Logger logger("openjerry::engine::http::EntryImpl");
} /* anonymous namespace */

EntryImpl::EntryImpl(std::unique_ptr<esl::processing::Procedure> aProcedure)
: procedure(std::move(aProcedure))
{ }

EntryImpl::EntryImpl(esl::processing::Procedure& aRefProcedure)
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

EntryImpl::EntryImpl(std::unique_ptr<esl::com::http::server::RequestHandler> aRequestHandler)
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
		esl::io::Input input = context->accept(requestContext);
		if(input) {
			return input;
		}
	}

	if(refContext) {
		/* ************************* *
		 * handle referenced context *
		 * ************************* */
		esl::io::Input input = refContext->accept(requestContext);
		if(input) {
			return input;
		}
	}

	if(host && host->isMatch(requestContext.getRequest().getHostName())) {
		/* *********** *
		 * handle host *
		 * *********** */
		esl::io::Input input = host->accept(requestContext);
		if(input) {
			return input;
		}
	}

#if 1
	if(endpoint) {
		logger.trace << "Check if endpoint '" << endpoint->getPath() << "' matches request path '" << requestContext.getPath() << "'\n";
		const char* subPath = endpoint->getMatch(requestContext.getPath());
		if(subPath) {
			logger.trace << "Match!\n";
			/* *************** *
			 * handle endpoint *
			 * *************** */
			std::string path = requestContext.getPath();

			requestContext.setPath(subPath);
			//requestContext.setPath(path.substr(endpoint->getPath().size()));
			logger.trace << "-> new request path is '" << requestContext.getPath() << "'\n";

			esl::io::Input input = endpoint->accept(requestContext);
			if(input) {
				logger.trace << "Request accepted by endpoint '" << endpoint->getPath() << "'\n";
				return input;
			}

			requestContext.setPath(path);
			logger.trace << "Request not accepted by endpoint '" << endpoint->getPath() << "' -> reset path to '" << requestContext.getPath() << "'\n";
		}
		else if(logger.trace) {
			logger.trace << "No match!\n";
		}
	}
#else
	if(endpoint && endpoint->isMatch(requestContext.getPath())) {
		logger.trace << "Request path '" << requestContext.getPath() << "' is matching endpoint with path '" << endpoint->getPath() << "'\n";
		/* *************** *
		 * handle endpoint *
		 * *************** */
		std::string path = requestContext.getPath();

		requestContext.setPath(endpoint->getMatchingSubPath(path));
		//requestContext.setPath(path.substr(endpoint->getPath().size()));
		logger.trace << "-> new request path is '" << requestContext.getPath() << "'\n";

		esl::io::Input input = endpoint->accept(requestContext);
		if(input) {
			logger.trace << "Request accepted by endpoint '" << endpoint->getPath() << "'\n";
			return input;
		}

		requestContext.setPath(path);
		logger.trace << "Request not accepted by endpoint '" << endpoint->getPath() << "' -> reset path to '" << requestContext.getPath() << "'\n";
	}
#endif
	if(requestHandler) {
		/* ********************** *
		 * handle request handler *
		 * ********************** */
		esl::io::Input input = requestHandler->accept(requestContext);
		if(input) {
			return input;
		}
	}

	return esl::io::Input();
}


} /* namespace http */
} /* namespace engine */
} /* namespace openjerry */
