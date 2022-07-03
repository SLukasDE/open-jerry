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

#include <jerry/engine/basic/Context.h>
#include <jerry/engine/basic/EntryImpl.h>
#include <jerry/Logger.h>

//#include <esl/Module.h>
#include <esl/object/InitializeContext.h>

#include <stdexcept>

namespace jerry {
namespace engine {
namespace basic {

namespace {
Logger logger("jerry::engine::basic::Context");
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

void Context::addContext(const std::string& refId) {
	Context* context = findObject<Context>(refId);

	if(context == nullptr) {
	    throw std::runtime_error("No context found with ref-id=\"" + refId + "\".");
	}

	entries.emplace_back(new EntryImpl(*context));
}

void Context::addContext(std::unique_ptr<Context> context) {
	entries.emplace_back(new EntryImpl(std::move(context)));
}

void Context::addRequestHandler(std::unique_ptr<esl::com::basic::server::RequestHandler> requestHandler) {
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

esl::io::Input Context::accept(esl::com::basic::server::RequestContext& requestContext) const {
	for(auto& entry : entries) {
		esl::io::Input input = entry->accept(requestContext);
		if(input) {
			return input;
		}
	}

	return esl::io::Input();
}

} /* namespace basic */
} /* namespace engine */
} /* namespace jerry */
