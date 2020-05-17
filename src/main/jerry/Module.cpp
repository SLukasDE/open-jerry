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

#include <jerry/Module.h>
#include <jerry/buildin/basicauth/RequestHandler.h>
#include <jerry/buildin/basicauth/Settings.h>
#include <jerry/buildin/filebrowser/RequestHandler.h>
#include <jerry/buildin/filebrowser/Settings.h>
#include <jerry/buildin/page/RequestHandler.h>
#include <jerry/buildin/page/Settings.h>

#include <esl/http/server/requesthandler/Interface.h>
#include <esl/object/Interface.h>
#include <esl/module/Interface.h>

#include <memory>
#include <new>         // placement new
#include <type_traits> // aligned_storage

namespace jerry {

namespace {
class Module : public esl::module::Module {
public:
	Module();
};

typename std::aligned_storage<sizeof(Module), alignof(Module)>::type moduleBuffer; // memory for the object;
Module* modulePtr = nullptr;


Module::Module()
: esl::module::Module()
{
	esl::module::Module::initialize(*this);

	addInterface(std::unique_ptr<const esl::module::Interface>(new esl::http::server::requesthandler::Interface(
			getId(), "jerry/buildin/basicauth", &jerry::buildin::basicauth::RequestHandler::create)));
	addInterface(std::unique_ptr<const esl::module::Interface>(new esl::object::Interface(
			getId(), "jerry/buildin/basicauth", &jerry::buildin::basicauth::Settings::create)));

	addInterface(std::unique_ptr<const esl::module::Interface>(new esl::http::server::requesthandler::Interface(
			getId(), "jerry/buildin/filebrowser", &jerry::buildin::filebrowser::RequestHandler::create)));
	addInterface(std::unique_ptr<const esl::module::Interface>(new esl::object::Interface(
			getId(), "jerry/buildin/filebrowser", &jerry::buildin::filebrowser::Settings::create)));

	addInterface(std::unique_ptr<const esl::module::Interface>(new esl::http::server::requesthandler::Interface(
			getId(), "jerry/buildin/page", &jerry::buildin::page::RequestHandler::create)));
	addInterface(std::unique_ptr<const esl::module::Interface>(new esl::object::Interface(
			getId(), "jerry/buildin/page", &jerry::buildin::page::Settings::create)));
}

} /* anonymous namespace */

esl::module::Module& getModule() {
	if(modulePtr == nullptr) {
		/* ***************** *
		 * initialize module *
		 * ***************** */

		modulePtr = reinterpret_cast<Module*> (&moduleBuffer);
		new (modulePtr) Module; // placement new
	}

	return *modulePtr;
}

} /* namespace jerry */
