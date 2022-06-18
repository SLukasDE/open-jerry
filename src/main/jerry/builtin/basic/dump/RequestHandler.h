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

#ifndef JERRY_BUILTIN_BASIC_DUMP_REQUESTHANDLER_H_
#define JERRY_BUILTIN_BASIC_DUMP_REQUESTHANDLER_H_

#include <esl/com/basic/server/requesthandler/Interface.h>
#include <esl/com/basic/server/RequestContext.h>
#include <esl/io/Input.h>

#include <memory>
#include <set>
#include <string>
#include <utility>
#include <vector>

namespace jerry {
namespace builtin {
namespace basic {
namespace dump {

class RequestHandler final : public esl::com::basic::server::requesthandler::Interface::RequestHandler {
public:
	static inline const char* getImplementation() {
		return "jerry/dump";
	}

	static std::unique_ptr<esl::com::basic::server::requesthandler::Interface::RequestHandler> createRequestHandler(const std::vector<std::pair<std::string, std::string>>& settings);

	RequestHandler(const std::vector<std::pair<std::string, std::string>>& settings);

	esl::io::Input accept(esl::com::basic::server::RequestContext& requestContext) const override;
	std::set<std::string> getNotifiers() const override;

private:
	bool showContext = true;
	bool showContent = false;
	std::string notifier;
};

} /* namespace dump */
} /* namespace basic */
} /* namespace builtin */
} /* namespace jerry */

#endif /* JERRY_BUILTIN_BASIC_DUMP_REQUESTHANDLER_H_ */
