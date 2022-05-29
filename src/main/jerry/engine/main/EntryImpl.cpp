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

#include <jerry/engine/main/EntryImpl.h>
#include <jerry/engine/main/Context.h>
#include <jerry/engine/basic/Context.h>
#include <jerry/Logger.h>

#include <esl/object/InitializeContext.h>

#include <string>

namespace jerry {
namespace engine {
namespace main {

namespace {
Logger logger("jerry::engine::main::EntryImpl");
} /* anonymous namespace */

EntryImpl::EntryImpl(std::unique_ptr<esl::processing::procedure::Interface::Procedure> aProcedure)
: procedure(std::move(aProcedure))
{ }

EntryImpl::EntryImpl(esl::processing::procedure::Interface::Procedure& aRefProcedure)
: refProcedure(&aRefProcedure)
{ }

EntryImpl::EntryImpl(std::unique_ptr<procedure::Context> aProcedureContext)
: procedureContext(std::move(aProcedureContext))
{ }

EntryImpl::EntryImpl(procedure::Context& aRefProcedureContext)
: refProcedureContext(&aRefProcedureContext)
{ }

EntryImpl::EntryImpl(std::unique_ptr<basic::Server> aBasicServer)
: basicServer(std::move(aBasicServer))
{ }

EntryImpl::EntryImpl(std::unique_ptr<http::Server> aHttpServer)
: httpServer(std::move(aHttpServer))
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

	if(procedureContext) {
		/* ****************** *
		 * initialize context *
		 * ****************** */
		procedureContext->initializeContext();
	}

	if(basicServer) {
		basicServer->initializeContext();
	}

	if(httpServer) {
		/* *********************************************************** *
		 * add certificates to socket if http-server is used for https *
		 * *********************************************************** */
		if(httpServer->isHttps()) {
			if(ownerContext.getCertificates().empty()) {
				throw std::runtime_error("No certificates are available.");
			}
			for(const auto& certificate : ownerContext.getCertificates()) {
				httpServer->addTLSHost(certificate.first, certificate.second.first, certificate.second.second);
			}
		}

		httpServer->initializeContext();
	}
}

void EntryImpl::procedureRun(esl::object::ObjectContext& objectContext) {
	if(procedure) {
		/* *************** *
		 * start procedure *
		 * *************** */
		logger.debug << "Start procedure ...\n";
		procedure->procedureRun(objectContext);
		logger.debug << "... procedure started.\n";
	}

	if(refProcedure) {
		/* ************************** *
		 * start referenced procedure *
		 * ************************** */
		logger.debug << "Start referenced procedure ...\n";
		refProcedure->procedureRun(objectContext);
		logger.debug << "... referenced procedure started.\n";
	}

	if(procedureContext) {
		/* *********************** *
		 * start procedure context *
		 * *********************** */
		logger.debug << "Start procedure context ...\n";
		procedureContext->procedureRun(objectContext);
		logger.debug << "... procedure context started.\n";
	}

	if(refProcedureContext) {
		/* ********************************** *
		 * start referenced procedure context *
		 * ********************************** */
		logger.debug << "Start referenced procedure context ...\n";
		refProcedureContext->procedureRun(objectContext);
		logger.debug << "... referenced procedure context started.\n";
	}

	if(basicServer) {
		/* ****************** *
		 * start basic server *
		 * ****************** */
		logger.debug << "Start basic server ...\n";
		if(logger.debug) {
			std::set<std::string> notifiers = basicServer->getContext().getNotifiers();
			for(const auto& notifier : notifiers) {
				logger.debug << "   - \"" << notifier << "\"\n";
			}
		}
		basicServer->procedureRun(objectContext);
		logger.debug << "... basic server started.\n";
	}

	if(httpServer) {
		/* ****************** *
		 * start basic server *
		 * ****************** */
		logger.debug << "Start HTTP/HTTPS server ...\n";
		httpServer->procedureRun(objectContext);
		logger.debug << "... HTTP/HTTPS server started.\n";
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

	if(basicServer) {
		/* ***************** *
		 * dump basic server *
		 * ***************** */
		for(std::size_t i=0; i<depth; ++i) {
			logger.info << "|   ";
		}
		logger.info << "+-> Basic server: -> " << basicServer.get() << "\n";
		basicServer->dumpTree(depth + 1);
	}

	if(httpServer) {
		/* **************** *
		 * dump HTTP server *
		 * **************** */
		for(std::size_t i=0; i<depth; ++i) {
			logger.info << "|   ";
		}
		logger.info << "+-> HTTP server: -> " << httpServer.get() << "\n";
		httpServer->dumpTree(depth + 1);
	}
}

void EntryImpl::setProcessRegistry(ProcessRegistry* processRegistry) {
	if(procedureContext) {
		procedureContext->setProcessRegistry(processRegistry);
	}
}

} /* namespace main */
} /* namespace engine */
} /* namespace jerry */