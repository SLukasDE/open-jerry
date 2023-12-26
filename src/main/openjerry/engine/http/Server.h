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

#ifndef OPENJERRY_ENGINE_HTTP_SERVER_H_
#define OPENJERRY_ENGINE_HTTP_SERVER_H_

#include <openjerry/engine/http/Context.h>
#include <openjerry/engine/http/RequestHandler.h>
#include <openjerry/engine/ProcessRegistry.h>

#include <esl/com/http/server/Socket.h>
#include <esl/object/Context.h>
#include <esl/object/Procedure.h>

#include <cstdint>
#include <memory>
#include <string>
#include <vector>
#include <utility>

namespace openjerry {
namespace engine {
namespace http {

class Server final : public esl::object::Procedure {
public:
	Server(ProcessRegistry& processRegistry, const std::vector<std::pair<std::string, std::string>>& settings, const std::string& implementation);

	void initializeContext();

	void procedureRun(esl::object::Context&) override;
	void procedureCancel() override;

	Context& getContext() noexcept;
	void dumpTree(std::size_t depth) const;

private:
	std::unique_ptr<esl::com::http::server::Socket> socket;
	ProcessRegistry& processRegistry;
	Context context;
	RequestHandler requestHandler;

	const std::string implementation;
	const std::vector<std::pair<std::string, std::string>> settings;
};

} /* namespace http */
} /* namespace engine */
} /* namespace openjerry */

#endif /* OPENJERRY_ENGINE_HTTP_SERVER_H_ */
