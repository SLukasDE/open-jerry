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
#include <esl/module/Interface.h>
#include <new>         // placement new
#include <type_traits> // aligned_storage

namespace jerry {

namespace {
class Module : public esl::module::Module {
public:
	Module();
};

typename std::aligned_storage<sizeof(Module), alignof(Module)>::type moduleBuffer; // memory for the object;
Module& myModule = reinterpret_cast<Module&> (moduleBuffer);
bool isInitialized = false;

Module::Module()
: esl::module::Module()
{
	esl::module::Module::initialize(*this);
}

} /* anonymous namespace */

esl::module::Module& getModule() {
	if(isInitialized == false) {
		/* ***************** *
		 * initialize module *
		 * ***************** */

		isInitialized = true;
		new (&myModule) Module; // placement new
	}
	return myModule;
}

} /* namespace jerry */
