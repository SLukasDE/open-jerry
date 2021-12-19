/*
 * This file is part of Jerry application server.
 * Copyright (C) 2020-2021 Sven Lukas
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

#include <jerry/engine/basic/server/Context.h>
#include <jerry/engine/basic/server/Listener.h>
#include <jerry/engine/basic/server/RequestContext.h>
#include <jerry/Logger.h>

#include <esl/Module.h>
#include <esl/com/basic/server/requesthandler/Interface.h>
#include <esl/object/InitializeContext.h>

#include <stdexcept>

namespace jerry {
namespace engine {
namespace basic {
namespace server {

namespace {
Logger logger("jerry::engine::messaging::server::Context");
} /* anonymous namespace */

/*
void Context::setParent(Context* context) {

}
*/

Context& Context::addContext(const std::string& id, bool inheritObjects) {
	std::unique_ptr<Context> context(new Context);
	Context& contextRef = *context;

	if(inheritObjects) {
		contextRef.setParent(this);
	}
	if(id == "") {
		entries.push_back(Entry(std::move(context)));
	}
	else {
		addObject(id, std::unique_ptr<esl::object::Interface::Object>(context.release()));
	}

	return contextRef;
}

void Context::addContext(const std::string& refId) {
	Context* context = findObject<Context>(refId);

	if(context == nullptr) {
	    throw std::runtime_error("No context found with ref-id=\"" + refId + "\".");
	}

	entries.push_back(Entry(*context));
}

void Context::addRequestHandler(const std::string& implementation, const esl::module::Interface::Settings& settings) {
	std::unique_ptr<esl::com::basic::server::requesthandler::Interface::RequestHandler> requestHandler;
	requestHandler = esl::getModule().getInterface<esl::com::basic::server::requesthandler::Interface>(implementation).createRequestHandler(settings);
	entries.push_back(Entry(std::move(requestHandler)));
}

void Context::initializeContext() {
	ObjectContext::initializeContext();

	// call initializeContext() of sub-context's
	for(auto& entry : entries) {
		if(entry.context) {
			/* ************** *
			 * handle context *
			 * ************** */
			entry.context->initializeContext();
		}
	}
}

void Context::dumpTree(std::size_t depth) const {
	ObjectContext::dumpTree(depth);

	for(auto& entry : entries) {
		if(entry.context) {
			/* ************** *
			 * handle Context *
			 * ************** */
			for(std::size_t i=0; i<depth; ++i) {
				logger.info << "|   ";
			}
			logger.info << "+-> Context:\n";
			entry.context->dumpTree(depth + 1);
		}

		if(entry.requestHandler) {
			/* ********************* *
			 * handle RequestHandler *
			 * ********************* */
			for(std::size_t i=0; i<depth; ++i) {
				logger.info << "|   ";
			}
			logger.info << "+-> BasicHandler\n";
		}
	}
}

std::set<std::string> Context::getNotifiers() const {
	std::set<std::string> notifiers;
logger.info << "create Notifiers for " << entries.size() << " entries.\n";
	for(auto& entry : entries) {
		if(entry.context) {
			std::set<std::string> tmpNotifiers = entry.context->getNotifiers();
logger.info << "- entry is a context with " << tmpNotifiers.size() << " notifiers.\n";
			notifiers.insert(tmpNotifiers.begin(), tmpNotifiers.end());
		}
		else if(entry.refContext) {
			std::set<std::string> tmpNotifiers = entry.refContext->getNotifiers();
logger.info << "- entry is a ref-context with " << tmpNotifiers.size() << " notifiers.\n";
			notifiers.insert(tmpNotifiers.begin(), tmpNotifiers.end());
		}
		else if(entry.requestHandler) {
			std::set<std::string> tmpNotifiers = entry.requestHandler->getNotifiers();
logger.info << "- entry is a request-handler with " << tmpNotifiers.size() << " notifiers.\n";
			notifiers.insert(tmpNotifiers.begin(), tmpNotifiers.end());
		}
else {
logger.info << "- entry is NOTHING.\n";
}
	}

	return notifiers;
}

esl::io::Input Context::accept(RequestContext& requestContext) const {
	esl::io::Input input;

	//requestContext->setParent(this);
	for(auto& entry : entries) {
		if(entry.context) {
			/* ************** *
			 * handle context *
			 * ************** */
			input = entry.context->accept(requestContext);
			if(input) {
				return input;
			}
			//requestContext->setParent(this);
		}

		if(entry.refContext) {
			/* ************************* *
			 * handle referenced context *
			 * ************************* */
			input = entry.refContext->accept(requestContext);
			if(input) {
				return input;
			}
			//requestContext->setParent(this);
		}

		if(entry.requestHandler) {
			/* **************************** *
			 * handle requestHandlerFactory *
			 * **************************** */

			input = entry.requestHandler->accept(requestContext, requestContext.getContext());
			if(input) {
				return input;
			}
			//requestContext->setParent(this);
		}
	}

	return input;
}

} /* namespace server */
} /* namespace basic */
} /* namespace engine */
} /* namespace jerry */
