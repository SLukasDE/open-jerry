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

#ifndef JERRY_BUILTIN_DAEMON_PROCEDURE_OBJECTCONTEXT_H_
#define JERRY_BUILTIN_DAEMON_PROCEDURE_OBJECTCONTEXT_H_

#include <esl/object/Interface.h>
#include <esl/object/ObjectContext.h>

#include <string>
#include <map>
#include <memory>

namespace jerry {
namespace builtin {
namespace daemon {
namespace procedure {

class ObjectContext final : public esl::object::ObjectContext {
public:
	void addObject(const std::string& id, std::unique_ptr<esl::object::Interface::Object> object) override;

protected:
	esl::object::Interface::Object* findRawObject(const std::string& id) override;
	const esl::object::Interface::Object* findRawObject(const std::string& id) const override;

private:
	std::map<std::string, std::unique_ptr<esl::object::Interface::Object>> objects;
};

} /* namespace procedure */
} /* namespace daemon */
} /* namespace builtin */
} /* namespace jerry */

#endif /* JERRY_BUILTIN_DAEMON_PROCEDURE_OBJECTCONTEXT_H_ */
