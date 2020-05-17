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

#ifndef JERRY_ENGINE_REQUESTCONTEXT_H_
#define JERRY_ENGINE_REQUESTCONTEXT_H_

#include <jerry/engine/Connection.h>

#include <esl/http/server/RequestContext.h>
#include <esl/http/server/Connection.h>
#include <esl/http/server/Request.h>
#include <esl/object/Interface.h>

#include <map>
#include <string>
#include <functional>

namespace jerry {
namespace engine {

class Context;

class RequestContext : public esl::http::server::RequestContext {
public:
	RequestContext(esl::http::server::RequestContext& baseRequestContext, const Context& engineContext);

	void setPath(std::string path);

	void setEngineContext(const Context& engineContext);
	const Context& getEngineContext() const;

	esl::http::server::Connection& getConnection() const override;
	const esl::http::server::Request& getRequest() const override;
	const std::string& getPath() const override;

	esl::object::Interface::Object* getObject(const std::string& id) const override;

private:
	esl::http::server::RequestContext& baseRequestContext;
	mutable Connection connection;
	std::reference_wrapper<const Context> engineContext;
	std::string path;
};

} /* namespace engine */
} /* namespace jerry */

#endif /* JERRY_ENGINE_REQUESTCONTEXT_H_ */
