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

#ifndef JERRY_BUILTIN_PROCEDURE_DETACH_PROCEDURE_H_
#define JERRY_BUILTIN_PROCEDURE_DETACH_PROCEDURE_H_

#include <jerry/engine/ProcessRegistry.h>

#include <esl/processing/procedure/Interface.h>
#include <esl/object/Interface.h>
#include <esl/object/InitializeContext.h>
#include <esl/object/ObjectContext.h>

#include <condition_variable>
#include <memory>
#include <mutex>
#include <string>
#include <utility>
#include <vector>

namespace jerry {
namespace builtin {
namespace procedure {
namespace detach {

class Procedure final : public virtual esl::processing::procedure::Interface::Procedure, public esl::object::InitializeContext {
public:
	static inline const char* getImplementation() {
		return "jerry/detach";
	}

	static std::unique_ptr<esl::processing::procedure::Interface::Procedure> create(const std::vector<std::pair<std::string, std::string>>& settings);

	Procedure(const std::vector<std::pair<std::string, std::string>>& settings);
	~Procedure();

	void procedureRun(esl::object::ObjectContext& objectContext) override;
	void procedureCancel() override;

	void initializeContext(esl::object::ObjectContext& objectContext) override;

private:
	std::string procedureId;
	esl::processing::procedure::Interface::Procedure* procedure = nullptr;
	engine::ProcessRegistry* processRegistry = nullptr;

	std::mutex runningProceduresMutex;
	std::size_t runningProcedures = 0;
	bool runningProceduresCancel = false;
	std::condition_variable runningProceduresCondVar;
};

} /* namespace detach */
} /* namespace procedure */
} /* namespace builtin */
} /* namespace jerry */

#endif /* JERRY_BUILTIN_PROCEDURE_DETACH_PROCEDURE_H_ */