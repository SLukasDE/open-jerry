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

#include <jerry/engine/basic/server/Context.h>
#include <jerry/engine/basic/server/RequestContext.h>
#include <jerry/engine/basic/server/EntryImpl.h>
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
Logger logger("jerry::engine::basic::server::Context");
} /* anonymous namespace */

Context& Context::addContext(const std::string& id, bool inheritObjects) {
	std::unique_ptr<Context> context(new Context);
	Context& contextRef = *context;

	if(inheritObjects) {
		contextRef.setParent(this);
	}
	if(id == "") {
		entries.emplace_back(new EntryImpl(std::move(context)));
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

	entries.emplace_back(new EntryImpl(*context));
}

void Context::addRequestHandler(const std::string& implementation, const esl::module::Interface::Settings& settings) {
	std::unique_ptr<esl::com::basic::server::requesthandler::Interface::RequestHandler> requestHandler;
	requestHandler = esl::getModule().getInterface<esl::com::basic::server::requesthandler::Interface>(implementation).createRequestHandler(settings);
	entries.emplace_back(new EntryImpl(std::move(requestHandler)));
}

void Context::initializeContext() {
	ObjectContext::initializeContext();

	// call initializeContext() of sub-context's
	for(auto& entry : entries) {
		entry->initializeContext(*this);
	}
}

void Context::dumpTree(std::size_t depth) const {
	ObjectContext::dumpTree(depth);

	for(auto& entry : entries) {
		entry->dumpTree(depth);
	}
}

std::set<std::string> Context::getNotifiers() const {
	std::set<std::string> notifiers;

	for(auto& entry : entries) {
		std::set<std::string> tmpNotifiers = entry->getNotifiers();
		notifiers.insert(tmpNotifiers.begin(), tmpNotifiers.end());
	}

	return notifiers;
}

esl::io::Input Context::accept(RequestContext& requestContext) const {
	//requestContext->setParent(this);
	for(auto& entry : entries) {
		esl::io::Input input = entry->accept(requestContext);
		if(input) {
			return input;
		}
		//requestContext->setParent(this);
	}

	return esl::io::Input();
}

} /* namespace server */
} /* namespace basic */
} /* namespace engine */
} /* namespace jerry */
