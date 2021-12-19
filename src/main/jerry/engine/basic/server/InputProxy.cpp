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

#include <jerry/engine/basic/server/InputProxy.h>
#include <jerry/Logger.h>

namespace jerry {
namespace engine {
namespace basic {
namespace server {

namespace {
Logger logger("jerry::engine::basic::server::InputProxy");
}

esl::io::Input InputProxy::create(esl::io::Input&& input, std::unique_ptr<RequestContext> requestContext) {
	std::unique_ptr<InputProxy> inputProxy(new InputProxy(std::move(input), std::move(requestContext)));
	esl::io::Consumer& consumer = inputProxy->getConsumer();
	esl::io::Writer& writer = inputProxy->getWriter();
	return esl::io::Input(std::unique_ptr<esl::object::Interface::Object>(inputProxy.release()), consumer, writer);
}

InputProxy::InputProxy(esl::io::Input&& aInput, std::unique_ptr<RequestContext> aRequestContext)
: engine::InputProxy(std::move(aInput)),
  requestContext(std::move(aRequestContext))
{ }

} /* namespace server */
} /* namespace basic */
} /* namespace engine */
} /* namespace jerry */
