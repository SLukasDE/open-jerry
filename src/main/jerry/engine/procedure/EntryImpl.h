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

#ifndef JERRY_ENGINE_PROCEDURE_ENTRYIMPL_H_
#define JERRY_ENGINE_PROCEDURE_ENTRYIMPL_H_

#include <jerry/engine/procedure/Entry.h>
#include <jerry/engine/procedure/Context.h>
#include <jerry/engine/ProcessRegistry.h>

#include <esl/processing/procedure/Interface.h>

#include <memory>

namespace jerry {
namespace engine {
namespace procedure {

class EntryImpl : public Entry {
public:
	EntryImpl(std::unique_ptr<esl::processing::procedure::Interface::Procedure> procedure);
	EntryImpl(esl::processing::procedure::Interface::Procedure& refProcedure);

	EntryImpl(std::unique_ptr<Context> context);
	EntryImpl(Context& refContext);

	void initializeContext(Context& ownerContext) override;
	void dumpTree(std::size_t depth) const override;

	void procedureRun(esl::object::ObjectContext& objectContext) override;
	void procedureCancel() override;

	void setProcessRegistry(ProcessRegistry* processRegistry) override;

private:
	std::unique_ptr<esl::processing::procedure::Interface::Procedure> procedure;
	esl::processing::procedure::Interface::Procedure* refProcedure = nullptr;

	std::unique_ptr<Context> context;
	Context* refContext = nullptr;
};

} /* namespace procedure */
} /* namespace engine */
} /* namespace jerry */

#endif /* JERRY_ENGINE_PROCEDURE_ENTRYIMPL_H_ */
