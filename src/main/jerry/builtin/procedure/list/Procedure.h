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

#ifndef JERRY_BUILTIN_PROCEDURE_LIST_PROCEDURE_H_
#define JERRY_BUILTIN_PROCEDURE_LIST_PROCEDURE_H_

#include <esl/processing/procedure/Interface.h>
#include <esl/object/InitializeContext.h>
#include <esl/object/ObjectContext.h>
#include <esl/module/Interface.h>

#include <string>
#include <vector>
#include <mutex>
#include <memory>


namespace jerry {
namespace builtin {
namespace procedure {
namespace list {

class Procedure final : public virtual esl::processing::procedure::Interface::Procedure, public esl::object::InitializeContext {
public:
	static inline const char* getImplementation() {
		return "jerry/list";
	}

	static std::unique_ptr<esl::processing::procedure::Interface::Procedure> createProcedure(const esl::module::Interface::Settings& settings);

	Procedure(const esl::module::Interface::Settings& settings);

	void initializeContext(esl::object::ObjectContext& objectContext) override;

	void procedureRun(esl::object::ObjectContext& objectContext) override;
	void procedureCancel() override;

private:
	std::vector<std::string> procedureIds;
	std::vector<esl::processing::procedure::Interface::Procedure*> procedures;

	std::mutex runningMutex;

	std::mutex currentProcedureMutex;
	esl::processing::procedure::Interface::Procedure* currentProcedure = nullptr;
	bool currentProcedureCancel = false;
};

} /* namespace list */
} /* namespace procedure */
} /* namespace builtin */
} /* namespace jerry */

#endif /* JERRY_BUILTIN_PROCEDURE_LIST_PROCEDURE_H_ */
