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

#include <jerry/engine/basic/server/EntryImpl.h>
#include <jerry/engine/basic/server/Context.h>
#include <jerry/Logger.h>

#include <esl/object/InitializeContext.h>

namespace jerry {
namespace engine {
namespace basic {
namespace server {

namespace {
Logger logger("jerry::engine::basic::server::EntryImpl");
} /* anonymous namespace */

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

esl::io::Input EntryImpl::accept(RequestContext& requestContext) {
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
		return requestHandler->accept(requestContext, requestContext.getContext());
	}

	return esl::io::Input();
}

} /* namespace server */
} /* namespace basic */
} /* namespace engine */
} /* namespace jerry */
