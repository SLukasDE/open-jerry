/*
 * This file is part of Jerry application server.
 * Copyright (C) 2020 Sven Lukas
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

#ifndef JERRY_ENGINE_REQUESTHANDLER_H_
#define JERRY_ENGINE_REQUESTHANDLER_H_

#include <jerry/engine/RequestContext.h>
#include <esl/http/server/requesthandler/Interface.h>
#include <esl/http/server/RequestContext.h>
//#include <esl/object/parameter/Interface.h>
#include <esl/Stacktrace.h>
#include <string>
#include <map>
#include <memory>
#include <functional>

namespace jerry {
namespace engine {

class Context;
class Endpoint;
class Listener;
//class RequestContext;

class RequestHandler : public esl::http::server::requesthandler::Interface::RequestHandler {
public:
	RequestHandler(const Listener& listener, esl::http::server::RequestContext& baseRequestContext);
	RequestHandler(std::unique_ptr<esl::http::server::RequestContext> requestContext, const Endpoint& engineEndpoint);

	void setContext(const Context& context);

	void setEndpoint(const Endpoint& endpoint);

	void setRequestHandler(esl::http::server::requesthandler::Interface::CreateRequestHandler createRequestHandler);
	bool hasRequestHandler() const;

	const std::vector<std::string>& getPathList();

	bool process(const char* contentData, std::size_t contentDataSize) override;

private:
	std::unique_ptr<esl::http::server::requesthandler::Interface::RequestHandler> requestHandler;
	RequestContext requestContext;

	std::reference_wrapper<const Endpoint> engineEndpoint;
	std::vector<std::string> pathList;
	bool isEndingWithSlash = false;

	std::string outputContent;
	std::size_t outputPos = 0;

	int getData(char* buffer, std::size_t count);

	void sendExceptionPage(std::string exceptionMsg, std::unique_ptr<esl::Stacktrace> stacktrace);

};

} /* namespace engine */
} /* namespace jerry */

#endif /* JERRY_ENGINE_REQUESTHANDLER_H_ */
