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

#ifndef JERRY_ENGINE_ENDPOINT_H_
#define JERRY_ENGINE_ENDPOINT_H_

#include <jerry/engine/Context.h>
#include <esl/http/server/RequestContext.h>
#include <string>
#include <vector>
#include <memory>

namespace jerry {
namespace engine {

class Listener;

class Endpoint : public Context {
public:
	Endpoint(Listener& listener, Context* parent, std::string path);

	std::unique_ptr<esl::http::server::RequestHandler> createRequestHandler(esl::http::server::RequestContext& requestContext, const std::string& path) const;

protected:
	std::vector<std::string> getEndpointPathList() const override;
	std::unique_ptr<esl::http::server::RequestHandler> createRequestHandler(esl::http::server::RequestContext& requestContext, const std::string& path, const Endpoint& endpoint) const override;

private:
	std::vector<std::string> pathList;
	std::vector<std::unique_ptr<Endpoint>> endpoints;
};

} /* namespace engine */
} /* namespace jerry */

#endif /* JERRY_ENGINE_ENDPOINT_H_ */
