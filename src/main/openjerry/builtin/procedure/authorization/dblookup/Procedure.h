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

#ifndef OPENJERRY_BUILTIN_PROCEDURE_AUTHORIZATION_DBLOOKUP_PROCEDURE_H_
#define OPENJERRY_BUILTIN_PROCEDURE_AUTHORIZATION_DBLOOKUP_PROCEDURE_H_

#include <esl/database/ConnectionFactory.h>
#include <esl/object/Context.h>
#include <esl/object/InitializeContext.h>
#include <esl/object/Procedure.h>
#include <esl/object/Value.h>

#include <map>
#include <memory>
#include <string>
#include <utility>
#include <vector>

namespace openjerry {
namespace builtin {
namespace procedure {
namespace authorization {
namespace dblookup {

class Procedure final : public virtual esl::object::Procedure, public esl::object::InitializeContext {
public:
	using Properties = esl::object::Value<std::map<std::string, std::string>>;

	static std::unique_ptr<esl::object::Procedure> create(const std::vector<std::pair<std::string, std::string>>& settings);

	Procedure(const std::vector<std::pair<std::string, std::string>>& settings);

	void procedureRun(esl::object::Context& objectContext) override;
	void procedureCancel() override { };

	void initializeContext(esl::object::Context& objectContext) override;

private:
	std::string authorizedObjectId = "authorized";
	std::string connectionId;
	std::string sql;
	esl::database::ConnectionFactory* connectionFactory = nullptr;
};

} /* namespace dblookup */
} /* namespace authorization */
} /* namespace procedure */
} /* namespace builtin */
} /* namespace openjerry */

#endif /* OPENJERRY_BUILTIN_PROCEDURE_AUTHORIZATION_DBLOOKUP_PROCEDURE_H_ */
