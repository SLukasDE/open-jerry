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

#ifndef JERRY_BUILTIN_OBJECT_APPLICATION_APPLICATION_H_
#define JERRY_BUILTIN_OBJECT_APPLICATION_APPLICATION_H_

#include <jerry/engine/ObjectContext.h>
#include <jerry/engine/basic/Context.h>
#include <jerry/engine/http/Context.h>

#include <esl/module/Library.h>
#include <esl/object/ObjectContext.h>

#include <string>
#include <memory>

namespace jerry {
namespace builtin {
namespace object {
namespace application {

class Application : public engine::ObjectContext {
public:
	Application(esl::object::ObjectContext& parentObjectContext/*, engine::basic::Context& basicContext, engine::http::Context& httpContext*/, const std::string& libraryFileStr);

	engine::basic::Context& addBasicListener();
	engine::basic::Context* getBasicListener();

	engine::http::Context& addHttpListener();
	engine::http::Context* getHttpListener();

	void initializeContext() override;
	//void dumpTree(std::size_t depth) const override;

private:
	esl::module::Library* library = nullptr;
	std::unique_ptr<engine::basic::Context> basicListener;
	std::unique_ptr<engine::http::Context> httpListener;
};

} /* namespace application */
} /* namespace object */
} /* namespace builtin */
} /* namespace jerry */

#endif /* JERRY_BUILTIN_OBJECT_APPLICATION_APPLICATION_H_ */
