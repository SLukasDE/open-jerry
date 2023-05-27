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

#ifndef OPENJERRY_OBJECTCONTEXT_H_
#define OPENJERRY_OBJECTCONTEXT_H_

#if 0
#include <esl/object/Object.h>
#include <esl/object/Context.h>

#include <map>
#include <memory>
#include <set>
#include <string>

namespace openjerry {

class ObjectContext final : public esl::object::Context {
public:
	std::set<std::string> getObjectIds() const override;

protected:
	void addRawObject(const std::string& id, std::unique_ptr<esl::object::Object> object) override;
	esl::object::Object* findRawObject(const std::string& id) override;
	const esl::object::Object* findRawObject(const std::string& id) const override;

private:
	std::map<std::string, std::unique_ptr<esl::object::Object>> objects;
};

} /* namespace openjerry */
#else

#include <common4esl/object/Context.h>

namespace openjerry {

using ObjectContext = common4esl::object::Context;

} /* namespace openjerry */

#endif

#endif /* OPENJERRY_OBJECTCONTEXT_H_ */
