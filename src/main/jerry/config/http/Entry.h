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

#ifndef JERRY_CONFIG_HTTP_ENTRY_H_
#define JERRY_CONFIG_HTTP_ENTRY_H_

#include <jerry/config/Config.h>
#include <jerry/config/Object.h>
#include <jerry/config/Reference.h>
#include <jerry/config/http/Context.h>
#include <jerry/config/http/Endpoint.h>
#include <jerry/config/http/Host.h>
#include <jerry/config/http/RequestHandler.h>
#include <jerry/engine/http/server/Context.h>

#include <memory>
#include <ostream>

#include <tinyxml2/tinyxml2.h>

namespace jerry {
namespace config {
namespace http {

class Entry : public Config {
public:
	Entry(const Entry&);
	Entry(const std::string& fileName, const tinyxml2::XMLElement& element);

	void save(std::ostream& oStream, std::size_t spaces) const;
	void install(engine::http::server::Context& engineHttpContext) const;

private:
	std::unique_ptr<Object> object;
	std::unique_ptr<Reference> reference;
	std::unique_ptr<Context> context;
	std::unique_ptr<Endpoint> endpoint;
	std::unique_ptr<Host> host;
	std::unique_ptr<RequestHandler> requestHandler;
};

} /* namespace http */
} /* namespace config */
} /* namespace jerry */

#endif /* JERRY_CONFIG_HTTP_ENTRY_H_ */
