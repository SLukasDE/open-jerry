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

#ifndef JERRY_ENGINE_PROCEDURE_CONTEXT_H_
#define JERRY_ENGINE_PROCEDURE_CONTEXT_H_

#include <jerry/engine/ObjectContext.h>
#include <jerry/engine/procedure/Entry.h>

#include <esl/processing/procedure/Interface.h>

#include <map>
#include <memory>
#include <mutex>
#include <string>
#include <vector>

namespace jerry {
namespace engine {
namespace procedure {

class Context : public ObjectContext {
public:
	using ObjectContext::ObjectContext;

	void addProcedure(std::unique_ptr<esl::processing::procedure::Interface::Procedure> procedure);
	void addProcedure(const std::string& refId);

	void addContext(std::unique_ptr<Context> context);
	void addContext(const std::string& refId);

	void initializeContext() override;
	void dumpTree(std::size_t depth) const override;

	void procedureRun(esl::object::Context& objectContext);
	void procedureCancel();

	void setProcessRegistry(ProcessRegistry* processRegistry) override;

private:
	Context* parent = nullptr;
	std::vector<std::unique_ptr<Entry>> entries;

	std::mutex runningProceduresMutex;
	std::map<Entry*, std::size_t> runningProcedures;
	bool runningProceduresCancel = false;
};

} /* namespace procedure */
} /* namespace engine */
} /* namespace jerry */

#endif /* JERRY_ENGINE_PROCEDURE_CONTEXT_H_ */
