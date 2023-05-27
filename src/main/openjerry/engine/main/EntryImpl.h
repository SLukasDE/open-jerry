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

#ifndef OPENJERRY_ENGINE_MAIN_ENTRYIMPL_H_
#define OPENJERRY_ENGINE_MAIN_ENTRYIMPL_H_

#include <openjerry/engine/main/Entry.h>
#include <openjerry/engine/main/Context.h>
#include <openjerry/engine/http/Server.h>
#include <openjerry/engine/procedure/Context.h>
#include <openjerry/engine/ProcessRegistry.h>

#include <esl/object/Context.h>
#include <esl/processing/Procedure.h>

#include <memory>

namespace openjerry {
namespace engine {
namespace main {

class EntryImpl : public Entry {
public:
	EntryImpl(std::unique_ptr<esl::processing::Procedure> procedure);
	EntryImpl(esl::processing::Procedure& refProcedure);

	EntryImpl(std::unique_ptr<procedure::Context> procedureContext);
	EntryImpl(procedure::Context& refProcedureContext);

	EntryImpl(std::unique_ptr<http::Server> httpServer);

	void initializeContext(Context& ownerContext) override;
	void procedureRun(esl::object::Context& objectContext) override;
	void dumpTree(std::size_t depth) const override;
	void setProcessRegistry(ProcessRegistry* processRegistry) override;

private:
	std::unique_ptr<esl::processing::Procedure> procedure;
	esl::processing::Procedure* refProcedure = nullptr;

	std::unique_ptr<procedure::Context> procedureContext;
	procedure::Context* refProcedureContext = nullptr;

	std::unique_ptr<http::Server> httpServer;
};

} /* namespace main */
} /* namespace engine */
} /* namespace openjerry */

#endif /* OPENJERRY_ENGINE_MAIN_ENTRYIMPL_H_ */
