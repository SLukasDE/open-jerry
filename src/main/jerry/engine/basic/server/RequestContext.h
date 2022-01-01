/*
 * This file is part of Jerry application server.
 * Copyright (C) 2020-2022 Sven Lukas
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

#ifndef JERRY_ENGINE_BASIC_SERVER_REQUESTCONTEXT_H_
#define JERRY_ENGINE_BASIC_SERVER_REQUESTCONTEXT_H_

#include <jerry/engine/basic/server/Context.h>

#include <esl/com/basic/server/RequestContext.h>
#include <esl/com/basic/server/Connection.h>
#include <esl/com/basic/server/Request.h>
#include <esl/com/basic/server/requesthandler/Interface.h>
#include <esl/object/Interface.h>
#include <esl/io/Input.h>

namespace jerry {
namespace engine {
namespace basic {
namespace server {

class RequestContext : public esl::com::basic::server::RequestContext {
public:
	RequestContext(esl::com::basic::server::RequestContext& requestContext);

	esl::com::basic::server::Connection& getConnection() const override;
	const esl::com::basic::server::Request& getRequest() const override;

	void setParent(Context* context);
	Context& getContext();
	const Context& getContext() const;

private:
	esl::com::basic::server::RequestContext& requestContext;
	Context context;
	//Context* baseContext = nullptr;
};

} /* namespace server */
} /* namespace basic */
} /* namespace engine */
} /* namespace jerry */

#endif /* JERRY_ENGINE_BASIC_SERVER_REQUESTCONTEXT_H_ */
