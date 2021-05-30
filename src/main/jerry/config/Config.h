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

#ifndef JERRY_CONFIG_CONFIG_H_
#define JERRY_CONFIG_CONFIG_H_

#include <jerry/config/Certificate.h>
#include <jerry/config/LoggerConfig.h>
#include <jerry/config/Object.h>
#include <jerry/config/Reference.h>
#include <jerry/config/OptionalBool.h>
#include <jerry/config/http/Server.h>
#include <jerry/config/http/Context.h>
#include <jerry/config/http/Listener.h>
#include <jerry/config/messaging/Broker.h>
#include <jerry/config/messaging/Server.h>
#include <jerry/config/messaging/Context.h>
#include <jerry/config/messaging/Listener.h>
//#include <jerry/engine/Engine.h>
//#include <jerry/engine/http/Listener.h>
//#include <jerry/engine/http/Endpoint.h>
//#include <jerry/engine/http/Context.h>

#include <esl/logging/Layout.h>
#include <esl/module/Library.h>

#include <tinyxml2/tinyxml2.h>

#include <vector>
#include <set>
#include <string>
#include <memory>
#include <ostream>

namespace jerry {
namespace config {

class Config {
public:
	void loadFile(const std::string& fileName);

	void loadLibraries();

	std::unique_ptr<esl::logging::Layout> createLayout() const;
	void setLogLevel() const;
	//void setEngine(engine::Engine& engine) const;

	void save(std::ostream& oStream) const;

	std::vector<std::pair<std::string, esl::module::Library*>> eslLibraries;
	std::vector<std::pair<std::string, esl::module::Library*>> libraries;
	std::vector<std::string> includes;
	std::vector<Certificate> certificates;
	LoggerConfig loggerConfig;
	std::vector<Object> objects;

	std::vector<http::Server> httpServers;
	std::vector<http::Context> httpContext;
	std::vector<http::Listener> httpListeners;

	std::vector<messaging::Broker> messageBrokers;
	std::vector<messaging::Server> messageServers;
	std::vector<messaging::Context> messageContext;
	std::vector<messaging::Listener> messageListeners;

	std::set<std::string> filesLoaded;

private:
	void parseInclude(const tinyxml2::XMLElement& element);
	void parseLibrary(const tinyxml2::XMLElement& element);
/*
	void addObjectToEngineBaseContext(engine::BaseContext& engineBaseContext, const Object& objects) const;
	void addReferenceToEngineHttpContext(engine::http::Context& engineHttpContext, const Reference& reference) const;
	void addHttpContextToEngineHttpContext(engine::http::Context& engineHttpContext, const http::Context& httpContext) const;
	void addHttpEndpointToEngineHttpContext(engine::http::Context& engineHttpContext, const http::Endpoint& httpEndpoint) const;
	void addHttpRequestHandlerToEngineHttpContext(engine::http::Context& engineHttpContext, const http::RequestHandler& httpRequestHandler) const;
	void addHttpResponseHeadersToEngineHttpEndpoint(engine::http::Endpoint& engineEndpoint, const std::vector<Setting>& responseHeaders) const;
	void addExceptionsToEngineHttpEndpoint(engine::http::Endpoint& engineEndpoint, const Exceptions& exceptions) const;

	void addHttpEntriesToEngineHttpContext(engine::http::Context& engineContext, const std::vector<http::Entry>& entries) const;

	void addHttpListenerToEngine(engine::Engine& engine, const http::Listener& configListener) const;
*/
};

} /* namespace config */
} /* namespace jerry */

#endif /* JERRY_CONFIG_CONFIG_H_ */
