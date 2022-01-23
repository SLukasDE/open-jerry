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

#ifndef JERRY_ENGINE_BASIC_CONTEXT_H_
#define JERRY_ENGINE_BASIC_CONTEXT_H_

#include <jerry/engine/ObjectContext.h>
#include <jerry/engine/basic/Entry.h>

#include <esl/com/basic/server/RequestContext.h>
#include <esl/io/Input.h>
#include <esl/module/Interface.h>
#include <esl/processing/procedure/Interface.h>

#include <string>
#include <set>
#include <vector>
#include <memory>

namespace jerry {
namespace engine {
namespace basic {

class Context : public ObjectContext {
public:

	void addApplications(const std::string& refId);

	void addProcedure(std::unique_ptr<esl::processing::procedure::Interface::Procedure> procedure);
	void addProcedure(const std::string& refId);

	Context& addContext(const std::string& id, bool inheritObjects);
	void addContext(const std::string& refId);

	void addRequestHandler(const std::string& implementation, const esl::module::Interface::Settings& settings);

	void initializeContext() override;
	void dumpTree(std::size_t depth) const override;
	std::set<std::string> getNotifiers() const;
	esl::io::Input accept(esl::com::basic::server::RequestContext& requestContext) const;

private:
	Context* parent = nullptr;
	std::vector<std::unique_ptr<Entry>> entries;
};

} /* namespace basic */
} /* namespace engine */
} /* namespace jerry */

#endif /* JERRY_ENGINE_BASIC_CONTEXT_H_ */
