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
#include <jerry/config/messaging/Listener.h>
#include <jerry/config/messaging/Context.h>
#include <jerry/config/messaging/Entry.h>
#include <jerry/config/messaging/RequestHandler.h>
#include <jerry/config/http/Listener.h>
#include <jerry/config/http/Endpoint.h>
#include <jerry/config/http/Context.h>
#include <jerry/config/http/Entry.h>
#include <jerry/config/http/RequestHandler.h>
#include <jerry/config/OptionalBool.h>

#include <jerry/engine/BaseContext.h>
#include <jerry/engine/messaging/server/Listener.h>
#include <jerry/engine/messaging/server/Context.h>
#include <jerry/engine/http/server/Listener.h>
#include <jerry/engine/http/server/Endpoint.h>
#include <jerry/engine/http/server/Context.h>

#include <jerry/Logger.h>

#include <esl/object/Interface.h>
#include <esl/Stacktrace.h>

#include <stdexcept>
#include <string>
#include <vector>


namespace jerry {
namespace config {

namespace {
Logger logger("jerry::config::Engine");

void add(engine::BaseContext& engineBaseContext, const Object& objects);

void add(engine::Engine& engine, const messaging::Listener& configMessageListener);
void add(engine::messaging::server::Context& engineMessageContext, const Reference& reference);
void add(engine::messaging::server::Context& engineMessageContext, const std::vector<messaging::Entry>& configMessageEntries);
void add(engine::messaging::server::Context& engineMessageContext, const messaging::Context& configMessageContext);
void add(engine::messaging::server::Context& engineMessageContext, const messaging::RequestHandler& configMessageRequestHandler);

void add(engine::Engine& engine, const http::Listener& configHttpListener);
void add(engine::http::server::Context& engineHttpContext, const Reference& reference);
void add(engine::http::server::Context& engineHttpContext, const std::vector<http::Entry>& configHttpEntries);
void add(engine::http::server::Context& engineHttpContext, const http::Context& httpContext);
void add(engine::http::server::Context& engineHttpContext, const http::Endpoint& httpEndpoint);
void add(engine::http::server::Context& engineHttpContext, const http::RequestHandler& httpRequestHandler);
void add(engine::http::server::Endpoint& engineEndpoint, const std::vector<Setting>& responseHeaders);
void add(engine::http::server::Endpoint& engineEndpoint, const Exceptions& exceptions);


void addSettingsToEslObject(esl::object::Interface::Object& object, const std::string& implementation, const std::vector<Setting>& settings) {
	if(settings.empty()) {
		return;
	}

	esl::object::Settings* settingsObject = dynamic_cast<esl::object::Settings*>(&object);
	if(settingsObject == nullptr) {
		throw esl::addStacktrace(std::runtime_error("Cannot add settings to simple object implementation \"" + implementation + "\""));
	}
	for(const auto& setting : settings) {
		settingsObject->addSetting(setting.key, setting.value);
	}
}



void add(engine::BaseContext& engineBaseContext, const Object& object) {
	esl::object::Interface::Object& eslObject = engineBaseContext.addObject(object.id, object.implementation);
	addSettingsToEslObject(eslObject, object.implementation, object.settings);
}



void add(engine::Engine& engine, const messaging::Listener& configMessageListener) {
	// TODO: last parameter is still missing in configMessageListener. So it's temporary hard coded to "true".
	engine::messaging::server::Listener& engineMessageListener = engine.addMessageListener(configMessageListener.refId, true /*configMessageListener.inherit*/);

	add(engineMessageListener, configMessageListener.entries);
}

void add(engine::messaging::server::Context& engineMessageContext, const Reference& reference) {
	engineMessageContext.addReference(reference.id, reference.refId);
}

void add(engine::messaging::server::Context& engineMessageContext, const std::vector<messaging::Entry>& configMessageEntries) {
	for(const auto& entry : configMessageEntries) {
		switch(entry.getType()) {
		case messaging::Entry::etObject:
			add(engineMessageContext, entry.getObject());
			break;
		case messaging::Entry::etReference:
			add(engineMessageContext, entry.getReference());
			break;
		case messaging::Entry::etContext:
			add(engineMessageContext, entry.getContext());
			break;
		case messaging::Entry::etRequestHandler:
			add(engineMessageContext, entry.getRequestHandler());
			break;
		default:
			logger.warn << "There is an entry with an unkown type\n";
			break;
		}
	}
}

void add(engine::messaging::server::Context& engineMessageContext, const messaging::Context& configMessageContext) {
	// TODO: last parameter is still missing in configMessageContext. So it's temporary hard coded to "true".
	engine::messaging::server::Context& newEngineMessageContext = engineMessageContext.addContext(true /*configMessageContext.inherit*/);
	add(newEngineMessageContext, configMessageContext.entries);
}

void add(engine::messaging::server::Context& engineMessageContext, const messaging::RequestHandler& configMessageRequestHandler) {
	if(configMessageRequestHandler.objectImplementation || configMessageRequestHandler.settings.size() > 0) {
		engine::messaging::server::Context& newEngineMessageContext = engineMessageContext.addContext(true);

		std::string objectImplementation;
		if(configMessageRequestHandler.objectImplementation) {
			objectImplementation = *configMessageRequestHandler.objectImplementation;
		}
		else {
			objectImplementation = configMessageRequestHandler.implementation;
		}
		esl::object::Interface::Object& engineObject = newEngineMessageContext.addObject("", objectImplementation);
		addSettingsToEslObject(engineObject, objectImplementation, configMessageRequestHandler.settings);

		newEngineMessageContext.addMessageHandler(configMessageRequestHandler.implementation);
	}
	else {
		engineMessageContext.addMessageHandler(configMessageRequestHandler.implementation);
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

void add(engine::http::server::Context& engineHttpContext, const http::RequestHandler& configHttpRequestHandler) {
	if(configHttpRequestHandler.objectImplementation.empty() == false || configHttpRequestHandler.settings.size() > 0) {
		engine::http::server::Context& newEngineContext = engineHttpContext.addContext(true);

		std::string objectImplementation;
		if(configHttpRequestHandler.objectImplementation.empty()) {
			objectImplementation = configHttpRequestHandler.implementation;
		}
		else {
			objectImplementation = configHttpRequestHandler.objectImplementation;
		}
		esl::object::Interface::Object& engineObject = newEngineContext.addObject("", objectImplementation);
		addSettingsToEslObject(engineObject, objectImplementation, configHttpRequestHandler.settings);

		newEngineContext.addRequestHandler(configHttpRequestHandler.implementation);
	}
	else {
		engineHttpContext.addRequestHandler(configHttpRequestHandler.implementation);
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


	for(const auto& messageBroker : config.messageBrokers) {
		std::vector<std::pair<std::string, std::string>> settings;

		for(const auto& setting : messageBroker.settings) {
			settings.push_back(std::make_pair(setting.key, setting.value));
		}

		engine.addMessageBroker(messageBroker.id, messageBroker.brokers, settings, messageBroker.implementation);
	}


	for(const auto& messageServer : config.messageServers) {
		std::vector<std::pair<std::string, std::string>> settings;

		for(const auto& setting : messageServer.settings) {
			settings.push_back(std::make_pair(setting.key, setting.value));
		}

		engine.addMessageServer(messageServer.id, messageServer.port, settings, messageServer.implementation);
	}


	for(const auto& httpServer : config.httpServers) {
		std::vector<std::pair<std::string, std::string>> settings;

		for(const auto& setting : httpServer.settings) {
			settings.push_back(std::make_pair(setting.key, setting.value));
		}

		engine.addHttpServer(httpServer.id, httpServer.port, httpServer.isHttps, settings, httpServer.implementation);
	}

	for(const auto& messageListener : config.messageListeners) {
		add(engine, messageListener);
	}

	for(const auto& httpListener : config.httpListeners) {
		add(engine, httpListener);
	}
}

} /* namespace config */
} /* namespace jerry */
