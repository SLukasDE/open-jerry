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

#ifndef JERRY_ENGINE_CONTEXT_H_
#define JERRY_ENGINE_CONTEXT_H_

#include <jerry/engine/BaseContext.h>
#include <esl/object/parameter/Interface.h>
#include <esl/http/server/RequestHandler.h>
#include <string>
#include <map>
#include <memory>
#include <vector>

namespace jerry {
namespace engine {

class Listener;
class Endpoint;

class Context : public BaseContext {
public:
	Context& addContext();
	Endpoint& addEndpoint(std::string path);
	void addRequestHandler(const std::string& implementation);
	void addReference(const std::string& id, const std::string& refId);

	esl::object::parameter::Interface::Object& addObject(const std::string& id, const std::string& implementation) override;
	esl::object::parameter::Interface::Object* getObject(const std::string& id) const override;

	virtual esl::object::parameter::Interface::Object* getObjectWithEngine(const std::string& id) const;

protected:
	Context(Listener& listener, Context* parent);

	virtual std::vector<std::string> getEndpointPathList() const;

	void registerContext(std::unique_ptr<Context>);

	virtual std::unique_ptr<esl::http::server::RequestHandler> createRequestHandler(esl::http::server::RequestContext& requestContext, const std::string& path, const Endpoint& endpoint) const;

private:
	Listener& listener;
	const Context* parent = nullptr;

	std::vector<std::unique_ptr<Context>> contextList;
	std::map<std::string, esl::object::parameter::Interface::Object*> allObjectsById;
	std::vector<esl::http::server::RequestHandler::Factory> requestHandlerFactories;
};

} /* namespace engine */
} /* namespace jerry */

#endif /* JERRY_ENGINE_CONTEXT_H_ */
