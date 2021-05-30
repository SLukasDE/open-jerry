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

#ifndef JERRY_ENGINE_HTTP_SERVER_LISTENER_H_
#define JERRY_ENGINE_HTTP_SERVER_LISTENER_H_

#include <jerry/engine/http/server/Endpoint.h>

#include <esl/http/server/RequestContext.h>
#include <esl/io/Input.h>
#include <esl/object/Interface.h>
#include <esl/object/ObjectContext.h>

#include <string>
#include <map>
#include <vector>
#include <memory>

namespace jerry {
namespace engine {
namespace http {
namespace server {

class Listener : public Endpoint {
public:
	Listener(esl::object::ObjectContext& engineContext, bool inheritObjects, const std::string& hostname, std::vector<std::string> refIds);

friend class Endpoint;

	esl::object::Interface::Object* findHiddenObject(const std::string& id) const override;
	void dumpTree(std::size_t depth) const override;

	const std::vector<std::string>& getRefIds() const;

	const std::string& getHostname() const;
	esl::io::Input createRequestHandler(esl::http::server::RequestContext& baseRequestContext);

private:
	struct EndpointEntry {
		EndpointEntry() = default;
		EndpointEntry(const Endpoint* endpoint);

		//std::size_t depth = 0;
		const Endpoint* endpoint = nullptr;
		std::map<std::string, std::unique_ptr<EndpointEntry>> entries;
	};

	esl::object::ObjectContext& engineContext;
	std::string hostname;
	std::vector<std::string> refIds;

	EndpointEntry rootEndpointEntry;

	void registerEndpoint(const Endpoint& endpoint);
};

} /* namespace server */
} /* namespace http */
} /* namespace engine */
} /* namespace jerry */

#endif /* JERRY_ENGINE_HTTP_SERVER_LISTENER_H_ */
