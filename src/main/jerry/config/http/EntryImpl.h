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

#ifndef JERRY_CONFIG_HTTP_ENTRYIMPL_H_
#define JERRY_CONFIG_HTTP_ENTRYIMPL_H_

#include <jerry/config/Config.h>
#include <jerry/config/Object.h>
#include <jerry/config/Reference.h>
#include <jerry/config/Procedure.h>
#include <jerry/config/basic/Client.h>
#include <jerry/config/http/Entry.h>
#include <jerry/config/http/Client.h>
#include <jerry/config/http/Context.h>
#include <jerry/config/http/Endpoint.h>
#include <jerry/config/http/Host.h>
#include <jerry/config/http/RequestHandler.h>
#include <jerry/engine/http/Context.h>

#include <memory>
#include <string>
#include <ostream>

#include <tinyxml2/tinyxml2.h>

namespace jerry {
namespace config {
namespace http {

class EntryImpl : public Entry {
public:
	EntryImpl(const Entry&) = delete;
	EntryImpl(const std::string& fileName, const tinyxml2::XMLElement& element);

	void save(std::ostream& oStream, std::size_t spaces) const override;
	void install(engine::http::Context& engineHttpContext) const override;

private:
	std::unique_ptr<Object> object;
	std::unique_ptr<Reference> reference;
	std::unique_ptr<Procedure> procedure;
	std::unique_ptr<Context> context;
	std::unique_ptr<Endpoint> endpoint;
	std::unique_ptr<Host> host;
	std::unique_ptr<RequestHandler> requestHandler;
	std::unique_ptr<basic::Client> basicClient;
	std::unique_ptr<Client> httpClient;
};

} /* namespace http */
} /* namespace config */
} /* namespace jerry */

#endif /* JERRY_CONFIG_HTTP_ENTRYIMPL_H_ */
