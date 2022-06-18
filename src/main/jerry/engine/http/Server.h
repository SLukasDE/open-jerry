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

#ifndef JERRY_ENGINE_HTTP_SERVER_H_
#define JERRY_ENGINE_HTTP_SERVER_H_

#include <jerry/engine/http/Context.h>
#include <jerry/engine/http/RequestHandler.h>
#include <jerry/engine/ProcessRegistry.h>

#include <esl/com/http/server/Socket.h>
#include <esl/object/Context.h>
#include <esl/processing/procedure/Interface.h>

#include <cstdint>
#include <string>
#include <vector>
#include <utility>

namespace jerry {
namespace engine {
namespace http {

class Server final : public esl::processing::procedure::Interface::Procedure {
public:
	Server(ProcessRegistry& processRegistry, bool https, const std::vector<std::pair<std::string, std::string>>& settings, const std::string& implementation);

	void initializeContext();

	void procedureRun(esl::object::Context&) override;
	void procedureCancel() override;

	void addTLSHost(const std::string& hostname, std::vector<unsigned char> certificate, std::vector<unsigned char> key);
	bool isHttps() const noexcept;

	Context& getContext() noexcept;
	void dumpTree(std::size_t depth) const;

private:
	esl::com::http::server::Socket socket;
	ProcessRegistry& processRegistry;
	Context context;
	RequestHandler requestHandler;

	const bool https;

	const std::string implementation;
	const std::vector<std::pair<std::string, std::string>> settings;
};

} /* namespace http */
} /* namespace engine */
} /* namespace jerry */

#endif /* JERRY_ENGINE_HTTP_SERVER_H_ */
