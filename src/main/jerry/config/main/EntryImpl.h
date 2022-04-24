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

#ifndef JERRY_CONFIG_MAIN_ENTRYIMPL_H_
#define JERRY_CONFIG_MAIN_ENTRYIMPL_H_

#include <jerry/config/Config.h>
#include <jerry/config/Object.h>
#include <jerry/config/Reference.h>
#include <jerry/config/Database.h>
#include <jerry/config/basic/Client.h>
#include <jerry/config/basic/Server.h>
#include <jerry/config/http/Client.h>
#include <jerry/config/http/Server.h>
#include <jerry/config/main/Entry.h>
#include <jerry/config/main/Procedure.h>
#include <jerry/config/main/ProcedureContext.h>
#include <jerry/config/main/BasicContext.h>
#include <jerry/config/main/HttpContext.h>

#include <memory>
#include <ostream>

#include <tinyxml2/tinyxml2.h>

namespace jerry {
namespace config {
namespace main {

class EntryImpl : public Entry {
public:
	EntryImpl(const std::string& fileName, const tinyxml2::XMLElement& element, bool isJBoot);

	void save(std::ostream& oStream, std::size_t spaces) const override;
	void install(engine::main::Context& context) const override;

private:
	std::unique_ptr<Object> object;
	std::unique_ptr<Reference> reference;
	std::unique_ptr<Database> database;

	std::unique_ptr<Procedure> procedure;
	std::unique_ptr<ProcedureContext> procedureContext;

	std::unique_ptr<basic::Client> basicClient;
	std::unique_ptr<BasicContext> basicContext;
	std::unique_ptr<basic::Server> basicServer;

	std::unique_ptr<http::Client> httpClient;
	std::unique_ptr<HttpContext> httpContext;
	std::unique_ptr<http::Server> httpServer;
};

} /* namespace main */
} /* namespace config */
} /* namespace jerry */

#endif /* JERRY_CONFIG_MAIN_ENTRYIMPL_H_ */
