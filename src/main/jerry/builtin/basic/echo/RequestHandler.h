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

#ifndef JERRY_BUILTIN_BASIC_ECHO_REQUESTHANDLER_H_
#define JERRY_BUILTIN_BASIC_ECHO_REQUESTHANDLER_H_

#include <esl/com/basic/server/requesthandler/Interface.h>
#include <esl/com/basic/server/RequestContext.h>
#include <esl/com/basic/client/Interface.h>
#include <esl/io/Input.h>
#include <esl/object/InitializeContext.h>
#include <esl/object/Interface.h>

#include <string>
#include <memory>
#include <set>
#include <vector>
#include <utility>

namespace jerry {
namespace builtin {
namespace basic {
namespace echo {

class RequestHandler final : public virtual esl::com::basic::server::requesthandler::Interface::RequestHandler, public esl::object::InitializeContext {
public:
	static inline const char* getImplementation() {
		return "jerry/builtin/basic/echo";
	}

	static std::unique_ptr<esl::com::basic::server::requesthandler::Interface::RequestHandler> createRequestHandler(const esl::module::Interface::Settings& settings);

	RequestHandler(const esl::module::Interface::Settings& settings);

	esl::io::Input accept(esl::com::basic::server::RequestContext& requestContext) const override;
	std::set<std::string> getNotifiers() const override;
	void initializeContext(esl::object::Interface::ObjectContext& objectContext) override;

private:
	std::string notifier;
	unsigned long msDelay = 0;
	std::string connectionFactoryId;
	esl::com::basic::client::Interface::ConnectionFactory* connectionFactory = nullptr;
};

} /* namespace echo */
} /* namespace basic */
} /* namespace builtin */
} /* namespace jerry */

#endif /* JERRY_BUILTIN_BASIC_ECHO_REQUESTHANDLER_H_ */
