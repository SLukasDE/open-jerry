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

#ifndef JERRY_ENGINE_HTTP_SERVER_REQUESTCONTEXT_H_
#define JERRY_ENGINE_HTTP_SERVER_REQUESTCONTEXT_H_

#include <jerry/engine/http/server/Connection.h>

#include <esl/com/http/server/RequestContext.h>
#include <esl/com/http/server/Connection.h>
#include <esl/com/http/server/Request.h>
#include <esl/com/http/server/requesthandler/Interface.h>
#include <esl/object/Interface.h>
#include <esl/io/Input.h>

#include <string>
#include <functional>

namespace jerry {
namespace engine {
namespace http {
namespace server {

class Context;
class Endpoint;
class Writer;

class RequestContext : public esl::com::http::server::RequestContext {
public:
	RequestContext(esl::com::http::server::RequestContext& baseRequestContext, Writer& writer, const Endpoint& endpoint);

	esl::io::Input createRequestHandler(std::unique_ptr<Writer>& writer, esl::com::http::server::requesthandler::Interface::CreateInput createRequestHandler);
	esl::io::Input& getInput();
	const esl::io::Input& getInput() const;

	esl::com::http::server::Connection& getConnection() const override;
	const esl::com::http::server::Request& getRequest() const override;

	void setPath(std::string path);
	const std::string& getPath() const override;
	const std::vector<std::string>& getPathList();

	void setContext(const Context& context);
	const Context& getContext() const noexcept;

	void setEndpoint(const Endpoint& endpoint);
	const Endpoint& getEndpoint() const noexcept;

protected:
	esl::object::Interface::Object* findObject(const std::string& id) const override;

private:
	esl::io::Input input;
	esl::com::http::server::RequestContext& baseRequestContext;
	Writer& writer;
	mutable Connection connection;
	std::reference_wrapper<const Context> context;
	std::reference_wrapper<const Endpoint> endpoint;
	std::string path;
	std::vector<std::string> pathList;
	bool isEndingWithSlash = false;
};

} /* namespace server */
} /* namespace http */
} /* namespace engine */
} /* namespace jerry */

#endif /* JERRY_ENGINE_HTTP_SERVER_REQUESTCONTEXT_H_ */
