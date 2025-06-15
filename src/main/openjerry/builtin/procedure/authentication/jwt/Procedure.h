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

#ifndef OPENJERRY_BUILTIN_PROCEDURE_AUTHENTICATION_JWT_PROCEDURE_H_
#define OPENJERRY_BUILTIN_PROCEDURE_AUTHENTICATION_JWT_PROCEDURE_H_

#include <esl/com/http/client/ConnectionFactory.h>
#include <esl/crypto/PublicKey.h>
#include <esl/object/Context.h>
#include <esl/object/InitializeContext.h>
#include <esl/object/Procedure.h>
#include <esl/object/Value.h>

#include <gtx/PublicKey.h>

#include <functional>
#include <map>
#include <memory>
#include <set>
#include <string>
#include <tuple>
#include <utility>
#include <vector>

namespace openjerry {
namespace builtin {
namespace procedure {
namespace authentication {
namespace jwt {

class Procedure final : public virtual esl::object::Procedure, public esl::object::InitializeContext {
public:
	static std::unique_ptr<esl::object::Procedure> create(const std::vector<std::pair<std::string, std::string>>& settings);

	Procedure(const std::vector<std::pair<std::string, std::string>>& settings);

	void initializeContext(esl::object::Context& objectContext) override;

	void procedureRun(esl::object::Context& objectContext) override;
	void procedureCancel() override;

private:
	using Properties = esl::object::Value<std::map<std::string, std::string>>;
	std::set<std::string> dropFields;
	std::map<std::string, std::string> overrideFields;
	std::set<std::string> jwksConnectionFactoryIds;
	std::map<std::string, std::reference_wrapper<esl::com::http::client::ConnectionFactory>> jwksConnectionFactories;

	std::map<std::string, std::pair<std::unique_ptr<gtx::PublicKey>, std::string>> publicKeyById;

	std::pair<gtx::PublicKey*, std::string> getPublicKeyById(const std::string& kid);
};

} /* namespace jwt */
} /* namespace authentication */
} /* namespace procedure */
} /* namespace builtin */
} /* namespace openjerry */

#endif /* OPENJERRY_BUILTIN_PROCEDURE_AUTHENTICATION_JWT_PROCEDURE_H_ */
