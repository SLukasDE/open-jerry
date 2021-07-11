/*
 * This file is part of Jerry application server.
 * Copyright (C) 2020-2021 Sven Lukas
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

#include <esl/object/Interface.h>
#include <esl/object/InitializeContext.h>
#include <esl/object/ObjectContext.h>
#include <esl/database/Interface.h>
#include <esl/database/Connection.h>

#include <string>
#include <memory>

#ifndef JERRY_BUILTIN_HTTP_DATABASE_SETTINGS_H_
#define JERRY_BUILTIN_HTTP_DATABASE_SETTINGS_H_

namespace jerry {
namespace builtin {
namespace http {
namespace database {

class Settings : public virtual esl::object::Interface::Object, public esl::object::InitializeContext {
public:
	Settings(const esl::object::Interface::Settings& settings);

	void initializeContext(esl::object::ObjectContext& objectContext) override;

	bool check() const;

	const std::string& getConnectionId() const;
	const std::string& getSQL() const;

private:
	std::unique_ptr<esl::database::Connection> createConnection() const;

	std::string connectionId;
	std::string sql;
	esl::database::Interface::ConnectionFactory* connectionFactory = nullptr;
};

} /* namespace database */
} /* namespace http */
} /* namespace builtin */
} /* namespace jerry */

#endif /* JERRY_BUILTIN_HTTP_DATABASE_SETTINGS_H_ */
