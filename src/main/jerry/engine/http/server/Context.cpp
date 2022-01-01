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

#include <jerry/engine/http/server/Context.h>
#include <jerry/engine/http/server/EntryImpl.h>
#include <jerry/engine/http/server/Endpoint.h>
#include <jerry/engine/http/server/Host.h>
#include <jerry/engine/http/server/RequestContext.h>
#include <jerry/engine/http/server/ExceptionHandler.h>
#include <jerry/Logger.h>

#include <esl/Module.h>
#include <esl/com/http/server/requesthandler/Interface.h>
#include <esl/object/InitializeContext.h>

#include <stdexcept>

namespace jerry {
namespace engine {
namespace http {
namespace server {

namespace {
Logger logger("jerry::engine::http::server::Context");
} /* anonymous namespace */

Context::Context(bool aFollowParentOnFind)
: followParentOnFind(aFollowParentOnFind)
{ }

void Context::setParent(Context* context) {
	parent = context;
	if(followParentOnFind) {
		ObjectContext::setParent(context);
	}
}

const Context* Context::getParent() const {
	return parent;
}

Context& Context::addContext(const std::string& id, bool inheritObjects) {
	std::unique_ptr<Context> context(new Context);
	Context& reference = *context;

	if(id == "") {
		entries.emplace_back(new EntryImpl(std::move(context)));
	}
	else {
		addObject(id, std::unique_ptr<esl::object::Interface::Object>(context.release()));
	}

	if(inheritObjects) {
		reference.setParent(this);
	}

	return reference;
}

void Context::addContext(const std::string& refId) {
	Context* context = findObject<Context>(refId);

	if(context == nullptr) {
	    throw std::runtime_error("No context found with ref-id=\"" + refId + "\".");
	}

	entries.emplace_back(new EntryImpl(*context));
}

Context& Context::addEndpoint(const std::string& path, bool inheritObjects) {
	std::unique_ptr<Endpoint> endpoint(new Endpoint(path));
	Context& reference = *endpoint;

	entries.emplace_back(new EntryImpl(std::move(endpoint)));

	if(inheritObjects) {
		reference.setParent(this);
	}

	return reference;
}

Context& Context::addHost(const std::string& serverName, bool inheritObjects) {
	std::unique_ptr<Host> host(new Host(serverName));
	Context& reference = *host;

	entries.emplace_back(new EntryImpl(std::move(host)));

	if(inheritObjects) {
		reference.setParent(this);
	}

	return reference;
}

void Context::addRequestHandler(const std::string& implementation, const esl::module::Interface::Settings& settings) {
	std::unique_ptr<esl::com::http::server::requesthandler::Interface::RequestHandler> requestHandler;
	requestHandler = esl::getModule().getInterface<esl::com::http::server::requesthandler::Interface>(implementation).createRequestHandler(settings);
	entries.emplace_back(new EntryImpl(std::move(requestHandler)));
}

void Context::setShowException(Context::OptionalBool aShowException) {
	showException = aShowException;
}

bool Context::getShowException() const {
	switch(showException) {
	case obTrue:
		return true;
	case obFalse:
		return false;
	default:
		break;
	}

	if(getParent()) {
		return getParent()->getShowException();
	}

	return true;
}

void Context::setShowStacktrace(Context::OptionalBool aShowStacktrace) {
	showStacktrace = aShowStacktrace;
}

bool Context::getShowStacktrace() const {
	switch(showStacktrace) {
	case obTrue:
		return true;
	case obFalse:
		return false;
	default:
		break;
	}

	if(getParent()) {
		return getParent()->getShowStacktrace();
	}

	return false;
}

void Context::setInheritErrorDocuments(bool aInheritErrorDocuments) {
	inheritErrorDocuments = aInheritErrorDocuments;
}

bool Context::getInheritErrorDocuments() const {
	return inheritErrorDocuments;
}

void Context::addErrorDocument(unsigned short statusCode, const std::string& path, bool parse) {
	if(errorDocuments.count(statusCode) > 0) {
        throw std::runtime_error("There are multiple error documents defined for http status code \"" + std::to_string(statusCode) + "\".");
	}

	Document document(path);
	document.setLanguage(parse ? Document::builtinScript : Document::none);
	errorDocuments.insert(std::make_pair(statusCode, document));
}

const Document* Context::findErrorDocument(unsigned short statusCode) const {
	const auto iter = errorDocuments.find(statusCode);

	if(iter == std::end(errorDocuments)) {
		if(getInheritErrorDocuments() && getParent()) {
			return getParent()->findErrorDocument(statusCode);
		}
		return nullptr;
	}

	return &iter->second;
}

void Context::addHeader(std::string key, std::string value) {
	headers.insert(std::make_pair(key, value));
}

const std::map<std::string, std::string>& Context::getHeaders() const {
	return headers;
}

const std::map<std::string, std::string>& Context::getEffectiveHeaders() const {
	return headersEffective;
}

void Context::initializeContext() {
	// initialize objects of this context
	ObjectContext::initializeContext();

	// build effective headers
	if(getParent()) {
		headersEffective = getParent()->getEffectiveHeaders();
		for(const auto& header : headers) {
			headersEffective[header.first] = header.second;
		}
	}
	else {
		headersEffective = headers;
	}

	// call initializeContext() of sub-context's
	for(auto& entry : entries) {
		entry->initializeContext(*this);
	}
}

void Context::dumpTree(std::size_t depth) const {
	if(showException != obEmpty) {
		for(std::size_t i=0; i<depth; ++i) {
			logger.info << "|   ";
		}
		logger.info << "showException: ";
		switch(showException) {
		case obTrue:
			logger.info << "true\n";
			break;
		case obFalse:
			logger.info << "false\n";
			break;
		default:
			logger.info << "\n";
			break;
		}
	}

	if(showStacktrace != obEmpty) {
		for(std::size_t i=0; i<depth; ++i) {
			logger.info << "|   ";
		}
		logger.info << "showStacktrace: ";
		switch(showStacktrace) {
		case obTrue:
			logger.info << "true\n";
			break;
		case obFalse:
			logger.info << "false\n";
			break;
		default:
			logger.info << "\n";
			break;
		}
	}

	for(std::size_t i=0; i<depth; ++i) {
		logger.info << "|   ";
	}
	logger.info << "inheritErrorDocuments: ";
	if(inheritErrorDocuments) {
		logger.info << "true\n";
	}
	else {
		logger.info << "false\n";
	}

	if(errorDocuments.empty() == false) {
		for(std::size_t i=0; i<depth; ++i) {
			logger.info << "|   ";
		}
		logger.info << "Documents for Status-Code:\n";
		for(const auto& statusCodeDocument : errorDocuments) {
			for(std::size_t i=0; i<depth; ++i) {
				logger.info << "|   ";
			}
			logger.info << "  " << statusCodeDocument.first << "\n";
		}
	}

	if(headers.empty() == false) {
		for(std::size_t i=0; i<depth; ++i) {
			logger.info << "|   ";
		}
		logger.info << "Additional response headers:\n";
		for(const auto& header : headers) {
			for(std::size_t i=0; i<depth; ++i) {
				logger.info << "|   ";
			}
			logger.info << "  \"" << header.first << "\"=\"" << header.second << "\"\n";
		}
	}

	ObjectContext::dumpTree(depth);

	// call dumpTree() of sub-context's
	for(auto& entry : entries) {
		entry->dumpTree(depth);
	}
}

esl::io::Input Context::accept(RequestContext& requestContext) {
	requestContext.setParentHeadersAndErrorHandlingContext(this);
	for(auto& entry : entries) {
		esl::io::Input input = entry->accept(requestContext);
		if(input) {
			return input;
		}
		requestContext.setParentHeadersAndErrorHandlingContext(this);
	}

	return esl::io::Input();
}

} /* namespace server */
} /* namespace http */
} /* namespace engine */
} /* namespace jerry */
