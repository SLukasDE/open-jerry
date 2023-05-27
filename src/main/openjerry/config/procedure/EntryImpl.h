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

#ifndef OPENJERRY_CONFIG_PROCEDURE_ENTRYIMPL_H_
#define OPENJERRY_CONFIG_PROCEDURE_ENTRYIMPL_H_

#include <openjerry/config/Config.h>
#include <openjerry/config/Object.h>
#include <openjerry/config/Reference.h>
#include <openjerry/config/Database.h>
#include <openjerry/config/http/Client.h>
#include <openjerry/config/procedure/Context.h>
#include <openjerry/config/procedure/Entry.h>
#include <openjerry/config/procedure/Procedure.h>

#include <memory>
#include <string>
#include <ostream>

#include <tinyxml2/tinyxml2.h>

namespace openjerry {
namespace config {
namespace procedure {

class EntryImpl : public Entry {
public:
	EntryImpl(const Entry&) = delete;
	EntryImpl(const std::string& fileName, const tinyxml2::XMLElement& element);

	void save(std::ostream& oStream, std::size_t spaces) const override;
	void install(engine::procedure::Context& engineContext) const override;

private:
	std::unique_ptr<Object> object;
	std::unique_ptr<Reference> reference;
	std::unique_ptr<Procedure> procedure;
	std::unique_ptr<Database> database;
	std::unique_ptr<Context> context;
	std::unique_ptr<http::Client> httpClient;
};

} /* namespace procedure */
} /* namespace config */
} /* namespace openjerry */

#endif /* OPENJERRY_CONFIG_PROCEDURE_ENTRYIMPL_H_ */
