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

#ifndef JERRY_CONFIG_PROCEDURECONTEXT_H_
#define JERRY_CONFIG_PROCEDURECONTEXT_H_

#include <jerry/config/Config.h>
#include <jerry/config/Setting.h>
#include <jerry/config/procedure/Entry.h>
#include <jerry/engine/procedure/Context.h>

#include <tinyxml2/tinyxml2.h>

#include <vector>
#include <string>
#include <ostream>
#include <memory>

namespace jerry {
namespace config {

class ProcedureContext : public Config {
public:
	ProcedureContext(const ProcedureContext&) = delete;
	ProcedureContext(const std::string& fileName, const tinyxml2::XMLElement& element);

	const std::string& getId() const noexcept;
	const std::string& getRefId() const noexcept;
	bool getInherit() const noexcept;

protected:
	void installEntries(engine::procedure::Context& newContext) const;
	void saveEntries(std::ostream& oStream, std::size_t spaces) const;

private:
	std::string id;
	std::string refId;

	bool inherit = true;
	std::vector<std::unique_ptr<procedure::Entry>> entries;

	void parseInnerElement(const tinyxml2::XMLElement& element);
};

} /* namespace config */
} /* namespace jerry */

#endif /* JERRY_CONFIG_PROCEDURECONTEXT_H_ */
