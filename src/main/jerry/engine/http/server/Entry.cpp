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

#include <jerry/engine/http/server/Entry.h>
#include <jerry/engine/http/server/Context.h>
#include <jerry/engine/http/server/Endpoint.h>

namespace jerry {
namespace engine {
namespace http {
namespace server {

Entry::Entry(std::unique_ptr<Context> aContext)
: context(std::move(aContext))
{ }

Entry::Entry(std::unique_ptr<Endpoint> aEndpoint)
: endpoint(std::move(aEndpoint))
{ }

Entry::Entry(esl::com::http::server::requesthandler::Interface::CreateInput aCreateInput)
: createRequestHandler(aCreateInput)
{ }

} /* namespace server */
} /* namespace http */
} /* namespace engine */
} /* namespace jerry */
