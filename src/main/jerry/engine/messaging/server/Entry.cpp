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

#include <jerry/engine/messaging/server/Entry.h>
#include <jerry/engine/messaging/server/Context.h>

namespace jerry {
namespace engine {
namespace messaging {
namespace server {

Entry::Entry(std::unique_ptr<Context> aContext)
: context(std::move(aContext))
{ }

Entry::Entry(const esl::messaging::server::requesthandler::Interface& aRequesthandler)
: requesthandler(&aRequesthandler)
{ }

} /* namespace server */
} /* namespace messaging */
} /* namespace engine */
} /* namespace jerry */