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

#include <jerry/config/Engine.h>
#include <jerry/config/Object.h>
#include <jerry/config/Reference.h>
#include <jerry/config/Setting.h>
#include <jerry/config/Exceptions.h>
#include <jerry/config/ExceptionDocument.h>
#include <jerry/config/basic/Listener.h>
#include <jerry/config/basic/Context.h>
#include <jerry/config/basic/Entry.h>
#include <jerry/config/basic/RequestHandler.h>
#include <jerry/config/http/Listener.h>
#include <jerry/config/http/Endpoint.h>
#include <jerry/config/http/Context.h>
#include <jerry/config/http/Entry.h>
#include <jerry/config/http/RequestHandler.h>
#include <jerry/config/OptionalBool.h>

#include <jerry/builtin/Properties.h>

#include <jerry/engine/BaseContext.h>
#include <jerry/engine/basic/server/Listener.h>
#include <jerry/engine/basic/server/Context.h>
#include <jerry/engine/http/server/Listener.h>
#include <jerry/engine/http/server/Endpoint.h>
#include <jerry/engine/http/server/Context.h>

#include <jerry/Logger.h>

#include <esl/object/Interface.h>
#include <esl/Stacktrace.h>

#include <stdexcept>
#include <vector>
#include <cstdlib>


namespace jerry {
namespace config {

namespace {
Logger logger("jerry::config::Engine");

void add(engine::BaseContext& engineBaseContext, const Object& objects);

void add(engine::Engine& engine, const basic::Listener& configBasicListener);
void add(engine::basic::server::Context& engineBasicContext, const Reference& reference);
void add(engine::basic::server::Context& engineBasicContext, const std::vector<basic::Entry>& configBasicEntries);
void add(engine::basic::server::Context& engineBasicContext, const basic::Context& configBasicContext);
void add(engine::basic::server::Context& engineBasicContext, const basic::RequestHandler& configBasicRequestHandler);

void add(engine::Engine& engine, const http::Listener& configHttpListener);
void add(engine::http::server::Context& engineHttpContext, const Reference& reference);
void add(engine::http::server::Context& engineHttpContext, const std::vector<http::Entry>& configHttpEntries);
void add(engine::http::server::Context& engineHttpContext, const http::Context& httpContext);
void add(engine::http::server::Context& engineHttpContext, const http::Endpoint& httpEndpoint);
void add(engine::http::server::Context& engineHttpContext, const http::RequestHandler& httpRequestHandler);
void add(engine::http::server::Endpoint& engineEndpoint, const std::vector<Setting>& responseHeaders);
void add(engine::http::server::Endpoint& engineEndpoint, const Exceptions& exceptions);



void add(engine::BaseContext& engineBaseContext, const Object& object) {
	std::vector<std::pair<std::string, std::string>> settings;

	for(const auto& setting : object.settings) {
		settings.push_back(std::make_pair(setting.key, Engine::evaluate(setting.value, setting.language)));
	}

	engineBaseContext.addObject(object.id, object.implementation, settings);
}



void add(engine::Engine& engine, const basic::Listener& configBasicListener) {
	// TODO: last parameter is still missing in configBasicListener. So it's temporary hard coded to "true".
	engine::basic::server::Listener& engineBasicListener = engine.addBasicListener(configBasicListener.refId, true /*configMessageListener.inherit*/);

	add(engineBasicListener, configBasicListener.entries);
}

void add(engine::basic::server::Context& engineBasicContext, const Reference& reference) {
	engineBasicContext.addReference(reference.id, reference.refId);
}

void add(engine::basic::server::Context& engineBasicContext, const std::vector<basic::Entry>& entries) {
	for(const auto& entry : entries) {
		switch(entry.getType()) {
		case basic::Entry::etObject:
			add(engineBasicContext, entry.getObject());
			break;
		case basic::Entry::etReference:
			add(engineBasicContext, entry.getReference());
			break;
		case basic::Entry::etContext:
			add(engineBasicContext, entry.getContext());
			break;
		case basic::Entry::etRequestHandler:
			add(engineBasicContext, entry.getRequestHandler());
			break;
		default:
			logger.warn << "There is an entry with an unkown type\n";
			break;
		}
	}
}

void add(engine::basic::server::Context& engineBasicContext, const basic::Context& configBasicContext) {
	// TODO: last parameter is still missing in configMessageContext. So it's temporary hard coded to "true".
	engine::basic::server::Context& newEngineMessageContext = engineBasicContext.addContext(true /*configMessageContext.inherit*/);
	add(newEngineMessageContext, configBasicContext.entries);
}

void add(engine::basic::server::Context& context, const basic::RequestHandler& configBasicRequestHandler) {
	std::string objectImplementation;
	if(configBasicRequestHandler.objectImplementation) {
		objectImplementation = *configBasicRequestHandler.objectImplementation;
	}
	else if(engine::BaseContext::hasObjectImplementation(configBasicRequestHandler.implementation)) {
		objectImplementation = configBasicRequestHandler.implementation;
	}
	else if(configBasicRequestHandler.settings.size() > 0) {
		objectImplementation = jerry::builtin::Properties::getImplementation();
	}

	if(objectImplementation.empty()) {
		if(configBasicRequestHandler.settings.size() > 0) {
			logger.warn << "empty object implementation for request handler \"" << configBasicRequestHandler.implementation << "\" but parameters have been specified.\n";
			logger.warn << "-> ignoring parameters.\n";
		}
		context.addRequestHandler(configBasicRequestHandler.implementation);
	}
	else {
		engine::basic::server::Context& newContext = context.addContext(true);

		std::vector<std::pair<std::string, std::string>> settings;
		for(const auto& setting : configBasicRequestHandler.settings) {
			settings.push_back(std::make_pair(setting.key, Engine::evaluate(setting.value, setting.language)));
		}

		newContext.addObject("", objectImplementation, settings);
		newContext.addRequestHandler(configBasicRequestHandler.implementation);
	}
}



void add(engine::Engine& engine, const http::Listener& configHttpListener) {
	engine::http::server::Listener& engineHttpListener = engine.addHttpListener(configHttpListener.refId, configHttpListener.inherit, configHttpListener.hostname);

	add(engineHttpListener, configHttpListener.entries);
	add(engineHttpListener, configHttpListener.responseHeaders);
	add(engineHttpListener, configHttpListener.exceptions);
}

void add(engine::http::server::Context& engineHttpContext, const Reference& reference) {
	engineHttpContext.addReference(reference.id, reference.refId);
}

void add(engine::http::server::Context& engineHttpContext, const std::vector<http::Entry>& configHttpEntries) {
	for(const auto& entry : configHttpEntries) {
		switch(entry.getType()) {
		case http::Entry::etObject:
			add(engineHttpContext, entry.getObject());
			break;
		case http::Entry::etReference:
			add(engineHttpContext, entry.getReference());
			break;
		case http::Entry::etEndpoint:
			add(engineHttpContext, entry.getEndpoint());
			break;
		case http::Entry::etContext:
			add(engineHttpContext, entry.getContext());
			break;
		case http::Entry::etRequestHandler:
			add(engineHttpContext, entry.getRequestHandler());
			break;
		default:
			logger.warn << "There is an entry with an unkown type\n";
			break;
		}
	}
}

void add(engine::http::server::Context& engineHttpContext, const http::Endpoint& configHttpEndpoint) {
	engine::http::server::Endpoint& newEngineHttpEndpoint = engineHttpContext.addEndpoint(configHttpEndpoint.path, configHttpEndpoint.inherit);

	add(newEngineHttpEndpoint, configHttpEndpoint.entries);

	add(newEngineHttpEndpoint, configHttpEndpoint.responseHeaders);
	add(newEngineHttpEndpoint, configHttpEndpoint.exceptions);
}

void add(engine::http::server::Context& engineHttpContext, const http::Context& configHttpContext) {
	engine::http::server::Context& newEngineHttpContext = engineHttpContext.addContext(configHttpContext.inherit);
	add(newEngineHttpContext, configHttpContext.entries);
}

void add(engine::http::server::Context& context, const http::RequestHandler& configHttpRequestHandler) {
	std::string objectImplementation;
	if(configHttpRequestHandler.objectImplementation.empty() == false) {
		objectImplementation = configHttpRequestHandler.objectImplementation;
	}
	else if(engine::BaseContext::hasObjectImplementation(configHttpRequestHandler.implementation)) {
		objectImplementation = configHttpRequestHandler.implementation;
	}
	else if(configHttpRequestHandler.settings.size() > 0) {
		objectImplementation = jerry::builtin::Properties::getImplementation();
	}

	if(objectImplementation.empty()) {
		context.addRequestHandler(configHttpRequestHandler.implementation);
	}
	else {
		engine::http::server::Context& newContext = context.addContext(true);

		std::vector<std::pair<std::string, std::string>> settings;
		for(const auto& setting : configHttpRequestHandler.settings) {
			settings.push_back(std::make_pair(setting.key, Engine::evaluate(setting.value, setting.language)));
		}

		newContext.addObject("", objectImplementation, settings);
		newContext.addRequestHandler(configHttpRequestHandler.implementation);
	}
}

void add(engine::http::server::Endpoint& engineHttpEndpoint, const std::vector<Setting>& responseHeaders) {
	for(const auto& responseHeader : responseHeaders) {
		engineHttpEndpoint.addHeader(responseHeader.key, responseHeader.value);
	}
}

void add(engine::http::server::Endpoint& engineHttpEndpoint, const Exceptions& exceptions) {
	/* set showExceptions */
	if(exceptions.showExceptions == OptionalBool::obTrue) {
		engineHttpEndpoint.setShowException(true);
	}
	else if(exceptions.showExceptions == OptionalBool::obFalse) {
		engineHttpEndpoint.setShowException(false);
	}

	/* set showStacktrace */
	if(exceptions.showStacktrace == OptionalBool::obTrue) {
		engineHttpEndpoint.setShowStacktrace(true);
	}
	else if(exceptions.showStacktrace == OptionalBool::obFalse) {
		engineHttpEndpoint.setShowStacktrace(false);
	}

	engineHttpEndpoint.setInheritErrorDocuments(exceptions.inheritDocuments);

	for(const auto& exceptionDocument : exceptions.documents) {
		engineHttpEndpoint.addErrorDocument(exceptionDocument.statusCode, exceptionDocument.path, exceptionDocument.parser);
	}
}

} /* anonymous namespace */

Engine::Engine(engine::Engine& aEngine)
: engine(aEngine)
{ }

void Engine::install(const Config& config) {
	for(const auto& configCertificate : config.certificates) {
		engine.addCertificate(configCertificate.domain, configCertificate.keyFile, configCertificate.certFile);
	}

	for(const auto& object : config.objects) {
		add(engine, object);
	}


	for(const auto& basicBroker : config.basicBrokers) {
		std::vector<std::pair<std::string, std::string>> settings;

		for(const auto& setting : basicBroker.settings) {
			settings.push_back(std::make_pair(setting.key, evaluate(setting.value, setting.language)));
		}

		engine.addBasicBroker(basicBroker.id, settings, basicBroker.implementation);
	}


	for(const auto& basicServer : config.basicServers) {
		std::vector<std::pair<std::string, std::string>> settings;

		for(const auto& setting : basicServer.settings) {
			settings.push_back(std::make_pair(setting.key, evaluate(setting.value, setting.language)));
		}

		engine.addBasicServer(basicServer.id, settings, basicServer.implementation);
	}


	for(const auto& httpServer : config.httpServers) {
		std::vector<std::pair<std::string, std::string>> settings;

		for(const auto& setting : httpServer.settings) {
			settings.push_back(std::make_pair(setting.key, evaluate(setting.value, setting.language)));
		}

		engine.addHttpServer(httpServer.id, httpServer.isHttps, settings, httpServer.implementation);
	}

	for(const auto& basicListener : config.basicListeners) {
		add(engine, basicListener);
	}

	for(const auto& httpListener : config.httpListeners) {
		add(engine, httpListener);
	}
}

std::string Engine::evaluate(const std::string& expression, const std::string& language) {
	if(language == "plain") {
		return expression;
	}

	std::string value;
	std::string var;
	enum {
		intro,
		begin,
		end
	} state = end;

	for(std::size_t i=0; i<expression.size(); ++i) {
		if(state == begin) {
			if(expression.at(i) == '}') {
				char* val = getenv(var.c_str());
				if(val == nullptr) {
					throw esl::addStacktrace(std::runtime_error("No value available for variable \"" + var + "\" in expression: \"" + expression + "\""));
				}
				value += val;
				state = end;
				var.clear();
			}
			else {
				var += expression.at(i);
			}
		}
		else if(state == intro) {
			if(expression.at(i) == '{') {
				state = begin;
			}
			else {
				throw esl::addStacktrace(std::runtime_error("Syntax error in expression: \"" + expression + "\""));
			}
		}
		else {
			if(expression.at(i) == '$') {
				state = intro;
			}
			else {
				value += expression.at(i);
			}
		}
	}

	return value;
}

} /* namespace config */
} /* namespace jerry */
