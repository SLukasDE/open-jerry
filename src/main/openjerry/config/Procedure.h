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

#ifndef OPENJERRY_CONFIG_PROCEDURE_H_
#define OPENJERRY_CONFIG_PROCEDURE_H_

#include <openjerry/config/Config.h>
#include <openjerry/config/Setting.h>

#include <esl/object/Procedure.h>

#include <tinyxml2.h>

#include <memory>
#include <ostream>
#include <string>
#include <vector>

namespace openjerry {
namespace config {

class Procedure : public Config {
public:
	Procedure(const std::string& fileName, const tinyxml2::XMLElement& element);

	void save(std::ostream& oStream, std::size_t spaces) const;

	const std::string& getId() const noexcept;
	const std::string& getRefId() const noexcept;

protected:
	std::unique_ptr<esl::object::Procedure> create() const;

private:
	std::string id;
	std::string implementation;
	std::string refId;
	std::vector<Setting> settings;

	void parseInnerElement(const tinyxml2::XMLElement& element);
};

} /* namespace config */
} /* namespace openjerry */

#endif /* OPENJERRY_CONFIG_PROCEDURE_H_ */
