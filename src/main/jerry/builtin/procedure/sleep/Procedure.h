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

#ifndef JERRY_BUILTIN_PROCEDURE_SLEEP_PROCEDURE_H_
#define JERRY_BUILTIN_PROCEDURE_SLEEP_PROCEDURE_H_

#include <esl/processing/procedure/Interface.h>
#include <esl/module/Interface.h>
#include <esl/object/Interface.h>

#include <memory>
#include <chrono>

namespace jerry {
namespace builtin {
namespace procedure {
namespace sleep {

class Procedure final : public esl::processing::procedure::Interface::Procedure {
public:
	static inline const char* getImplementation() {
		return "jerry/builtin/procedure/sleep";
	}

	static std::unique_ptr<esl::processing::procedure::Interface::Procedure> createProcedure(const esl::module::Interface::Settings& settings);
	static std::unique_ptr<esl::object::Interface::Object> createObject(const esl::module::Interface::Settings& settings);

	Procedure(const esl::module::Interface::Settings& settings);

	void procedureRun(esl::object::Interface::ObjectContext& objectContext) override;
	void procedureCancel() override { };

private:
	std::chrono::milliseconds sleepMs = std::chrono::milliseconds(0);
};

} /* namespace sleep */
} /* namespace procedure */
} /* namespace builtin */
} /* namespace jerry */

#endif /* JERRY_BUILTIN_PROCEDURE_SLEEP_PROCEDURE_H_ */
