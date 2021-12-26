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

#include <jerry/engine/http/server/Context.h>
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
		entries.push_back(Entry(std::move(context)));
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

	entries.push_back(Entry(*context));
}

Context& Context::addEndpoint(const std::string& path, bool inheritObjects) {
	std::unique_ptr<Endpoint> endpoint(new Endpoint(path));
	Context& reference = *endpoint;

	entries.push_back(Entry(std::move(endpoint)));

	if(inheritObjects) {
		reference.setParent(this);
	}

	return reference;
}

Context& Context::addHost(const std::string& serverName, bool inheritObjects) {
	std::unique_ptr<Host> host(new Host(serverName));
	Context& reference = *host;

	entries.push_back(Entry(std::move(host)));

	if(inheritObjects) {
		reference.setParent(this);
	}

	return reference;
}

void Context::addRequestHandler(const std::string& implementation, const esl::module::Interface::Settings& settings) {
	std::unique_ptr<esl::com::http::server::requesthandler::Interface::RequestHandler> requestHandler;
	requestHandler = esl::getModule().getInterface<esl::com::http::server::requesthandler::Interface>(implementation).createRequestHandler(settings);
	entries.push_back(Entry(std::move(requestHandler)));
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
		if(entry.context) {
			/* ****************** *
			 * initialize context *
			 * ****************** */
			entry.context->initializeContext();
		}

		if(entry.host) {
			/* *************** *
			 * initialize host *
			 * *************** */
			entry.host->initializeContext();
		}

		if(entry.endpoint) {
			/* ******************* *
			 * initialize endpoint *
			 * ******************* */
			entry.endpoint->initializeContext();
		}

		if(entry.requestHandler) {
			/* ************************* *
			 * initialize requestHandler *
			 * ************************* */
			esl::object::InitializeContext* initializeContext = dynamic_cast<esl::object::InitializeContext*>(entry.requestHandler.get());
			if(initializeContext) {
				initializeContext->initializeContext(*this);
			}
		}
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
		if(entry.context) {
			/* ************ *
			 * dump context *
			 * ************ */
			for(std::size_t i=0; i<depth; ++i) {
				logger.info << "|   ";
			}
			logger.info << "+-> Context: -> " << entry.context.get() << "\n";
			entry.context->dumpTree(depth + 1);
		}

		if(entry.refContext) {
			/* *********************** *
			 * dump referenced context *
			 * *********************** */
			for(std::size_t i=0; i<depth; ++i) {
				logger.info << "|   ";
			}
			logger.info << "+-> Context: -> " << entry.refContext << " (reference)\n";
		}

		if(entry.host) {
			/* ********* *
			 * dump host *
			 * ********* */
			for(std::size_t i=0; i<depth; ++i) {
				logger.info << "|   ";
			}
			logger.info << "+-> Host: -> " << entry.host.get() << "\n";
			entry.host->dumpTree(depth + 1);
		}

		if(entry.endpoint) {
			/* ************* *
			 * dump endpoint *
			 * ************* */
			for(std::size_t i=0; i<depth; ++i) {
				logger.info << "|   ";
			}
			logger.info << "+-> Endpoint: -> " << entry.endpoint.get() << "\n";
			entry.endpoint->dumpTree(depth + 1);
		}

		if(entry.requestHandler) {
			/* ******************* *
			 * dump requestHandler *
			 * ******************* */
			for(std::size_t i=0; i<depth; ++i) {
				logger.info << "|   ";
			}
			logger.info << "+-> RequestHandler: -> " << entry.requestHandler.get() << "\n";
		}
	}
}

esl::io::Input Context::accept(RequestContext& requestContext) {
	esl::io::Input input;

	requestContext.setParent(this);
	for(auto& entry : entries) {
		if(entry.context) {
			/* *************** *
			 * handle context *
			 * *************** */
			input = entry.context->accept(requestContext);
			if(input) {
				break;
			}
			requestContext.setParent(this);
		}

		if(entry.refContext) {
			/* ************************* *
			 * handle referenced context *
			 * ************************* */
			input = entry.refContext->accept(requestContext);
			if(input) {
				break;
			}
			requestContext.setParent(this);
		}

		if(entry.host) {
			/* *********** *
			 * handle host *
			 * *********** */
			if(entry.host->isMatch(requestContext.getRequest().getHostName())) {
				input = entry.host->accept(requestContext);
				if(input) {
					break;
				}
				requestContext.setParent(this);
			}
		}

		if(entry.endpoint) {
			/* *************** *
			 * handle endpoint *
			 * *************** */
			if(entry.endpoint->isMatch(requestContext.getPath())) {
				std::string path = requestContext.getPath();
				requestContext.setPath(path.substr(entry.endpoint->getPath().size()));
				input = entry.endpoint->accept(requestContext);
				if(input) {
					break;
				}
				requestContext.setParent(this);
				requestContext.setPath(path);
			}
		}

		if(entry.requestHandler) {
			/* ********************** *
			 * handle request handler *
			 * ********************** */
			input = entry.requestHandler->accept(requestContext, requestContext.getContext());
			if(input) {
				break;
			}
			requestContext.setParent(this);
		}
	}

	return input;
}

} /* namespace server */
} /* namespace http */
} /* namespace engine */
} /* namespace jerry */
