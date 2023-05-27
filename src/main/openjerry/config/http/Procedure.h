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

#ifndef OPENJERRY_CONFIG_HTTP_PROCEDURE_H_
#define OPENJERRY_CONFIG_HTTP_PROCEDURE_H_

#include <openjerry/config/Procedure.h>
#include <openjerry/engine/http/Context.h>

namespace openjerry {
namespace config {
namespace http {

class Procedure : public config::Procedure {
public:
	using config::Procedure::Procedure;

	void install(engine::http::Context& engineHttpContext) const;
};

} /* namespace http */
} /* namespace config */
} /* namespace openjerry */

#endif /* OPENJERRY_CONFIG_HTTP_PROCEDURE_H_ */
