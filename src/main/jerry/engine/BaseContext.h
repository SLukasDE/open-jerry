/*
 * This file is part of Jerry application server.
 * Copyright (C) 2020 Sven Lukas
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

#ifndef JERRY_ENGINE_BASECONTEXT_H_
#define JERRY_ENGINE_BASECONTEXT_H_

#include <esl/http/server/ObjectContext.h>
#include <esl/object/Interface.h>
#include <string>
#include <map>
#include <vector>
#include <memory>

namespace jerry {
namespace engine {

class BaseContext : public esl::http::server::ObjectContext {
public:
	virtual ~BaseContext() = default;

	virtual esl::object::Interface::Object& addObject(const std::string& id, const std::string& implementation);
	esl::object::Interface::Object* getObject(const std::string& id) const override;

	virtual void initializeContext();

protected:

private:
	std::map<std::string, std::unique_ptr<esl::object::Interface::Object>> localObjectsById;
};

} /* namespace engine */
} /* namespace jerry */

#endif /* JERRY_ENGINE_BASECONTEXT_H_ */
