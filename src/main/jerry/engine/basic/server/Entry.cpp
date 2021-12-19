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

#include <jerry/engine/basic/server/Entry.h>
#include <jerry/engine/basic/server/Context.h>

namespace jerry {
namespace engine {
namespace basic {
namespace server {

Entry::Entry(std::unique_ptr<Context> aContext)
: context(std::move(aContext))
{ }

Entry::Entry(Context& aRefContext)
: refContext(&aRefContext)
{ }

Entry::Entry(std::unique_ptr<esl::com::basic::server::requesthandler::Interface::RequestHandler> aRequestHandler)
: requestHandler(std::move(aRequestHandler))
{ }

} /* namespace server */
} /* namespace basic */
} /* namespace engine */
} /* namespace jerry */
