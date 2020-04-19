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

#ifndef JERRY_ENGINE_LISTENER_H_
#define JERRY_ENGINE_LISTENER_H_

#include <jerry/engine/Endpoint.h>
#include <esl/http/server/RequestContext.h>
#include <esl/http/server/requesthandler/Interface.h>
#include <cstdint>
#include <string>
#include <map>
#include <memory>
#include <vector>

namespace jerry {
namespace engine {

class Engine;

class Listener : public Endpoint {
friend class Endpoint;
public:
	Listener(Engine& engine);

	esl::object::Interface::Object* getHiddenObject(const std::string& id) const override;

	std::unique_ptr<esl::http::server::requesthandler::Interface::RequestHandler> createRequestHandler(esl::http::server::RequestContext& requestContext);

private:
	struct EndpointEntry {
		EndpointEntry() = default;
		EndpointEntry(const Endpoint* endpoint);

		//std::size_t depth = 0;
		const Endpoint* endpoint = nullptr;
		std::map<std::string, std::unique_ptr<EndpointEntry>> entries;
	};

	Engine& engine;

	EndpointEntry rootEndpointEntry;

	void registerEndpoint(const Endpoint& endpoint);
};

} /* namespace engine */
} /* namespace jerry */

#endif /* JERRY_ENGINE_LISTENER_H_ */
