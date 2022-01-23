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

#ifndef JERRY_BUILTIN_PROCEDURE_AUTHORIZATION_DBLOOKUP_PROCEDURE_H_
#define JERRY_BUILTIN_PROCEDURE_AUTHORIZATION_DBLOOKUP_PROCEDURE_H_

#include <esl/processing/procedure/Interface.h>
#include <esl/module/Interface.h>
#include <esl/database/Interface.h>
#include <esl/object/ObjectContext.h>
#include <esl/object/InitializeContext.h>
#include <esl/object/Value.h>

#include <map>
#include <memory>
#include <string>

namespace jerry {
namespace builtin {
namespace procedure {
namespace authorization {
namespace dblookup {

class Procedure final : public virtual esl::processing::procedure::Interface::Procedure, public esl::object::InitializeContext {
public:
	using Properties = esl::object::Value<std::map<std::string, std::string>>;

	static inline const char* getImplementation() {
		return "jerry/authorization-dblookup";
	}

	static std::unique_ptr<esl::processing::procedure::Interface::Procedure> createProcedure(const esl::module::Interface::Settings& settings);

	Procedure(const esl::module::Interface::Settings& settings);

	void procedureRun(esl::object::ObjectContext& objectContext) override;
	void procedureCancel() override { };

	void initializeContext(esl::object::ObjectContext& objectContext) override;

private:
	std::string authorizedObjectId = "authorized";
	std::string connectionId;
	std::string sql;
	esl::database::Interface::ConnectionFactory* connectionFactory = nullptr;
};

} /* namespace dblookup */
} /* namespace authorization */
} /* namespace procedure */
} /* namespace builtin */
} /* namespace jerry */

#endif /* JERRY_BUILTIN_PROCEDURE_AUTHORIZATION_DBLOOKUP_PROCEDURE_H_ */
