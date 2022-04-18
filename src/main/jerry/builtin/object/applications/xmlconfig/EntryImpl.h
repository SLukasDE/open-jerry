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

#ifndef JERRY_BUILTIN_OBJECT_APPLICATIONS_XMLCONFIG_ENTRYIMPL_H_
#define JERRY_BUILTIN_OBJECT_APPLICATIONS_XMLCONFIG_ENTRYIMPL_H_

#include <jerry/builtin/object/applications/Application.h>
#include <jerry/builtin/object/applications/xmlconfig/Entry.h>
#include <jerry/builtin/object/applications/xmlconfig/BasicContext.h>
#include <jerry/builtin/object/applications/xmlconfig/HttpContext.h>
#include <jerry/builtin/object/applications/xmlconfig/Procedure.h>
#include <jerry/builtin/object/applications/xmlconfig/ProcedureContext.h>
#include <jerry/config/Object.h>
#include <jerry/config/Reference.h>
#include <jerry/config/Database.h>
#include <jerry/config/basic/Client.h>
#include <jerry/config/http/Client.h>

#include <ostream>
#include <string>
#include <memory>

#include <tinyxml2/tinyxml2.h>

namespace jerry {
namespace builtin {
namespace object {
namespace applications {
namespace xmlconfig {

class EntryImpl : public Entry {
public:
	EntryImpl(const EntryImpl&) = delete;
	EntryImpl(const std::string& fileName, const tinyxml2::XMLElement& element);

	void save(std::ostream& oStream, std::size_t spaces) const override;
	void install(Application& engineApplicationContext) const override;

private:
	std::unique_ptr<config::Object> object;
	std::unique_ptr<config::Reference> reference;
	std::unique_ptr<config::Database> database;

	std::unique_ptr<Procedure> procedure;
	std::unique_ptr<ProcedureContext> procedureContext;

	std::unique_ptr<config::basic::Client> basicClient;
	std::unique_ptr<BasicContext> basicContext;

	std::unique_ptr<config::http::Client> httpClient;
	std::unique_ptr<HttpContext> httpContext;
};

} /* namespace xmlconfig */
} /* namespace applications */
} /* namespace object */
} /* namespace builtin */
} /* namespace jerry */

#endif /* JERRY_BUILTIN_OBJECT_APPLICATIONS_XMLCONFIG_ENTRYIMPL_H_ */
