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

#include <jerry/Module.h>

/* *************** *
 * jerry procedure *
 * *************** */
#include <jerry/engine/main/Context.h>

/* ****************************** *
 * builtin basic request handlers *
 * ****************************** */
#include <jerry/builtin/basic/dump/RequestHandler.h>
#include <jerry/builtin/basic/echo/RequestHandler.h>
#include <jerry/builtin/basic/application/RequestHandler.h>

/* ***************************** *
 * builtin http request handlers *
 * ***************************** */
#include <jerry/builtin/http/applications/RequestHandler.h>
#include <jerry/builtin/http/authentication/RequestHandler.h>
#include <jerry/builtin/http/database/RequestHandler.h>
#include <jerry/builtin/http/dump/RequestHandler.h>
#include <jerry/builtin/http/filebrowser/RequestHandler.h>
#include <jerry/builtin/http/file/RequestHandler.h>
#include <jerry/builtin/http/log/RequestHandler.h>
#include <jerry/builtin/http/self/RequestHandler.h>
#include <jerry/builtin/http/proxy/RequestHandler.h>

/* ****************** *
 * builtin procedures *
 * ****************** */
#include <jerry/builtin/procedure/authentication/basic/dblookup/Procedure.h>
#include <jerry/builtin/procedure/authentication/basic/stable/Procedure.h>
#include <jerry/builtin/procedure/authentication/jwt/Procedure.h>
#include <jerry/builtin/procedure/authorization/cache/Procedure.h>
#include <jerry/builtin/procedure/authorization/dblookup/Procedure.h>
#include <jerry/builtin/procedure/authorization/jwt/Procedure.h>
#include <jerry/builtin/procedure/detach/Procedure.h>
#include <jerry/builtin/procedure/list/Procedure.h>
#include <jerry/builtin/procedure/returncode/Procedure.h>
#include <jerry/builtin/procedure/sleep/Procedure.h>

/* *************** *
 * builtin objects *
 * *************** */
#include <jerry/builtin/object/standard/Int.h>
#include <jerry/builtin/object/standard/MapStringString.h>
#include <jerry/builtin/object/standard/SetInt.h>
#include <jerry/builtin/object/standard/SetString.h>
#include <jerry/builtin/object/standard/String.h>
#include <jerry/builtin/object/standard/VectorInt.h>
#include <jerry/builtin/object/standard/VectorPairStringString.h>
#include <jerry/builtin/object/standard/VectorString.h>
#include <jerry/builtin/object/applications/Object.h>

#include <eslx/Module.h>

#include <esl/com/basic/server/requesthandler/Interface.h>
#include <esl/com/http/server/requesthandler/Interface.h>
#include <esl/processing/procedure/Interface.h>
#include <esl/object/Interface.h>
#include <esl/logging/appender/Interface.h>
#include <esl/logging/layout/Interface.h>
#include <esl/Module.h>

namespace jerry {

void Module::install(esl::module::Module& module) {
	esl::setModule(module);

	eslx::Module::install(module);

	/* ***************************** *
	 * builtin basic request handlers *
	 * ***************************** */

	module.addInterface(esl::com::basic::server::requesthandler::Interface::createInterface(
			"jerry/dump", // builtin::basic::dump::RequestHandler::getImplementation(),
			&builtin::basic::dump::RequestHandler::createRequestHandler));

	module.addInterface(esl::com::basic::server::requesthandler::Interface::createInterface(
			"jerry/echo", // builtin::basic::echo::RequestHandler::getImplementation(),
			&builtin::basic::echo::RequestHandler::createRequestHandler));

	module.addInterface(esl::com::basic::server::requesthandler::Interface::createInterface(
			"jerry/applications", // builtin::basic::application::RequestHandler::getImplementation(),
			&builtin::basic::application::RequestHandler::create));

	/* ***************************** *
	 * builtin http request handlers *
	 * ***************************** */
	module.addInterface(esl::com::http::server::requesthandler::Interface::createInterface(
			"jerry/applications", // builtin::http::applications::RequestHandler::getImplementation(),
			&builtin::http::applications::RequestHandler::create));

	module.addInterface(esl::com::http::server::requesthandler::Interface::createInterface(
			"jerry/authentication", // builtin::http::authentication::RequestHandler::getImplementation(),
			&builtin::http::authentication::RequestHandler::createRequestHandler));

	module.addInterface(esl::com::http::server::requesthandler::Interface::createInterface(
			"jerry/database", // builtin::http::database::RequestHandler::getImplementation(),
			&builtin::http::database::RequestHandler::createRequestHandler));

	module.addInterface(esl::com::http::server::requesthandler::Interface::createInterface(
			"jerry/dump", // builtin::http::dump::RequestHandler::getImplementation(),
			&builtin::http::dump::RequestHandler::createRequestHandler));

	module.addInterface(esl::com::http::server::requesthandler::Interface::createInterface(
			"jerry/filebrowser", // builtin::http::filebrowser::RequestHandler::getImplementation(),
			&builtin::http::filebrowser::RequestHandler::createRequestHandler));

	module.addInterface(esl::com::http::server::requesthandler::Interface::createInterface(
			"jerry/file", // builtin::http::file::RequestHandler::getImplementation(),
			&builtin::http::file::RequestHandler::createRequestHandler));

	module.addInterface(esl::com::http::server::requesthandler::Interface::createInterface(
			"jerry/log", // builtin::http::file::RequestHandler::getImplementation(),
			&builtin::http::log::RequestHandler::createRequestHandler));

	module.addInterface(esl::com::http::server::requesthandler::Interface::createInterface(
			"jerry/self", // builtin::http::self::RequestHandler::getImplementation(),
			&builtin::http::self::RequestHandler::createRequestHandler));

	module.addInterface(esl::com::http::server::requesthandler::Interface::createInterface(
			"jerry/proxy", // builtin::http::proxy::RequestHandler::getImplementation(),
			&builtin::http::proxy::RequestHandler::createRequestHandler));

	/* ****************** *
	 * builtin procedures *
	 * ****************** */
	module.addInterface(esl::processing::procedure::Interface::createInterface(
			"jerry/authentication-basic-dblookup", // builtin::procedure::authentication::basic::dblookup::Procedure::getImplementation(),
			&builtin::procedure::authentication::basic::dblookup::Procedure::create));

	module.addInterface(esl::processing::procedure::Interface::createInterface(
			"jerry/authentication-basic-stable", // builtin::procedure::authentication::basic::stable::Procedure::getImplementation(),
			&builtin::procedure::authentication::basic::stable::Procedure::create));

	module.addInterface(esl::processing::procedure::Interface::createInterface(
			"jerry/authentication-jwt", // builtin::procedure::authentication::jwt::Procedure::getImplementation(),
			&builtin::procedure::authentication::jwt::Procedure::create));

	module.addInterface(esl::processing::procedure::Interface::createInterface(
			"jerry/authorization-cache", // builtin::procedure::authorization::cache::Procedure::getImplementation(),
			&builtin::procedure::authorization::cache::Procedure::create));

	module.addInterface(esl::processing::procedure::Interface::createInterface(
			"jerry/authorization-dblookup", // builtin::procedure::authorization::dblookup::Procedure::getImplementation(),
			&builtin::procedure::authorization::dblookup::Procedure::create));

	module.addInterface(esl::processing::procedure::Interface::createInterface(
			"jerry/authorization-jwt", // builtin::procedure::authorization::jwt::Procedure::getImplementation(),
			&builtin::procedure::authorization::jwt::Procedure::create));

	module.addInterface(esl::processing::procedure::Interface::createInterface(
			"jerry/detach", // builtin::procedure::list::Procedure::getImplementation(),
			&builtin::procedure::detach::Procedure::create));

	module.addInterface(esl::processing::procedure::Interface::createInterface(
			"jerry/list", // builtin::procedure::list::Procedure::getImplementation(),
			&builtin::procedure::list::Procedure::create));

	module.addInterface(esl::processing::procedure::Interface::createInterface(
			"jerry/return-code", // builtin::procedure::returncode::Procedure::getImplementation(),
			&builtin::procedure::returncode::Procedure::create));

	module.addInterface(esl::processing::procedure::Interface::createInterface(
			"jerry/sleep", // builtin::procedure::sleep::Procedure::getImplementation(),
			&builtin::procedure::sleep::Procedure::create));

	module.addInterface(esl::processing::procedure::Interface::createInterface(
			"jerry", // Procedure::getImplementation(),
			&engine::main::Context::create));

	/* *************** *
	 * builtin objects *
	 * *************** */
	module.addInterface(esl::object::Interface::createInterface(
			"std/int",
			&builtin::object::standard::Int::create));

	module.addInterface(esl::object::Interface::createInterface(
			"std/map<string,string>",
			&builtin::object::standard::MapStringString::create));

	module.addInterface(esl::object::Interface::createInterface(
			"std/set<int>",
			&builtin::object::standard::SetInt::create));

	module.addInterface(esl::object::Interface::createInterface(
			"std/set<string>",
			&builtin::object::standard::SetString::create));

	module.addInterface(esl::object::Interface::createInterface(
			"std/string",
			&builtin::object::standard::String::create));

	module.addInterface(esl::object::Interface::createInterface(
			"std/vector<int>",
			&builtin::object::standard::VectorInt::create));

	module.addInterface(esl::object::Interface::createInterface(
			"std/vector<pair<string,string>>",
			&builtin::object::standard::VectorPairStringString::create));

	module.addInterface(esl::object::Interface::createInterface(
			"std/vector<string>",
			&builtin::object::standard::VectorString::create));

	module.addInterface(esl::object::Interface::createInterface(
			"jerry/applications",
			&builtin::object::applications::Object::create));



	/* ************************* *
	 * builtin logging appenders *
	 * ************************* */
	module.addInterface(esl::logging::appender::Interface::createInterface(
			"jerry/membuffer",
			esl::getModule().getInterface<esl::logging::appender::Interface>("eslx/membuffer").createAppender));

	module.addInterface(esl::logging::appender::Interface::createInterface(
			"jerry/ostream",
			esl::getModule().getInterface<esl::logging::appender::Interface>("eslx/ostream").createAppender));

	/* *********************** *
	 * builtin logging layouts *
	 * *********************** */
	module.addInterface(esl::logging::layout::Interface::createInterface(
			"jerry/default",
			esl::getModule().getInterface<esl::logging::layout::Interface>("eslx/default").createLayout));

}

} /* namespace jerry */
