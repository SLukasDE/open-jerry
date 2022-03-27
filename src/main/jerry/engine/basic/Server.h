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

#ifndef JERRY_ENGINE_BASIC_SERVER_H_
#define JERRY_ENGINE_BASIC_SERVER_H_

#include <jerry/engine/basic/Context.h>
#include <jerry/engine/basic/RequestHandler.h>
#include <jerry/engine/ProcessRegistry.h>

#include <esl/com/basic/server/Socket.h>
#include <esl/object/ObjectContext.h>
#include <esl/processing/procedure/Interface.h>

#include <cstdint>
#include <string>
#include <vector>
#include <utility>
#include <set>

namespace jerry {
namespace engine {
namespace basic {

class Server final : public esl::processing::procedure::Interface::Procedure {
public:
	Server(ProcessRegistry& processRegistry, const std::vector<std::pair<std::string, std::string>>& settings, const std::string& implementation);

	void initializeContext();

	void procedureRun(esl::object::ObjectContext&) override;
	void procedureCancel() override;

	Context& getContext() noexcept;
	void dumpTree(std::size_t depth) const;

private:
	esl::com::basic::server::Socket socket;
	RequestHandler requestHandler;

	const std::string implementation;
	const std::vector<std::pair<std::string, std::string>> settings;
	ProcessRegistry& processRegistry;
	Context context;
};

} /* namespace basic */
} /* namespace engine */
} /* namespace jerry */

#endif /* JERRY_ENGINE_BASIC_SERVER_H_ */
