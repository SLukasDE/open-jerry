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
#include <esl/object/Interface.h>
#include <esl/http/server/requesthandler/Interface.h>
#include <string>
#include <map>
#include <memory>
#include <vector>
#include <tuple>

namespace jerry {
namespace engine {

class Listener;
class Endpoint;
class RequestHandler;

class Context : public BaseContext {
public:
	void addReference(const std::string& id, const std::string& refId);
	esl::object::Interface::Object& addObject(const std::string& id, const std::string& implementation) override;
	esl::object::Interface::Object* getObject(const std::string& id) const override;
	virtual esl::object::Interface::Object* getHiddenObject(const std::string& id) const;

	Context& addContext();
	Endpoint& addEndpoint(std::string path);
	void addRequestHandler(const std::string& implementation);

	const Endpoint& getEndpoint() const;

	void initializeContext() override;

protected:
	Context(Listener& listener, const Endpoint& endpoint, const Context& parentContext);

	// only used by Listener
	Context(Listener& listener);

	bool createRequestHandler(RequestHandler& requestHandler) const;

private:
	Listener& listener;
	const Endpoint& endpoint;
	const Context* parentContext = nullptr;

	std::map<std::string, esl::object::Interface::Object*> allObjectsById;

	std::vector<std::tuple<std::unique_ptr<Context>, std::unique_ptr<Endpoint>, esl::http::server::requesthandler::Interface::CreateRequestHandler>> contextCreateRequestHandlerList;

	esl::object::Interface::Object* getLocalObject(const std::string& id) const;
};

} /* namespace engine */
} /* namespace jerry */

#endif /* JERRY_ENGINE_CONTEXT_H_ */
