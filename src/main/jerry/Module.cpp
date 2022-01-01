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
#include <jerry/builtin/Properties.h>

#include <jerry/builtin/basic/dump/RequestHandler.h>
#include <jerry/builtin/basic/echo/RequestHandler.h>
#include <jerry/builtin/http/basicauth/RequestHandler.h>
#include <jerry/builtin/http/database/RequestHandler.h>
#include <jerry/builtin/http/filebrowser/RequestHandler.h>
#include <jerry/builtin/http/file/RequestHandler.h>
#include <jerry/builtin/http/self/RequestHandler.h>
#include <jerry/builtin/http/proxy/RequestHandler.h>
#include <jerry/builtin/procedure/sleep/Procedure.h>
#include <jerry/builtin/daemon/procedures/Daemon.h>

#include <esl/com/basic/server/requesthandler/Interface.h>
#include <esl/com/http/server/requesthandler/Interface.h>
#include <esl/processing/procedure/Interface.h>
#include <esl/processing/daemon/Interface.h>
#include <esl/object/Interface.h>
#include <esl/Module.h>

namespace jerry {

void Module::install(esl::module::Module& module) {
	esl::setModule(module);

	module.addInterface(esl::object::Interface::createInterface(
			builtin::Properties::getImplementation(),
			&builtin::Properties::createSettings));

	module.addInterface(esl::com::basic::server::requesthandler::Interface::createInterface(
			builtin::basic::dump::RequestHandler::getImplementation(),
			&builtin::basic::dump::RequestHandler::createRequestHandler));

	module.addInterface(esl::com::basic::server::requesthandler::Interface::createInterface(
			builtin::basic::echo::RequestHandler::getImplementation(),
			&builtin::basic::echo::RequestHandler::createRequestHandler));

	module.addInterface(esl::com::http::server::requesthandler::Interface::createInterface(
			builtin::http::basicauth::RequestHandler::getImplementation(),
			&builtin::http::basicauth::RequestHandler::createRequestHandler));

	module.addInterface(esl::com::http::server::requesthandler::Interface::createInterface(
			builtin::http::database::RequestHandler::getImplementation(),
			&builtin::http::database::RequestHandler::createRequestHandler));

	module.addInterface(esl::com::http::server::requesthandler::Interface::createInterface(
			builtin::http::filebrowser::RequestHandler::getImplementation(),
			&builtin::http::filebrowser::RequestHandler::createRequestHandler));

	module.addInterface(esl::com::http::server::requesthandler::Interface::createInterface(
			builtin::http::file::RequestHandler::getImplementation(),
			&builtin::http::file::RequestHandler::createRequestHandler));

	module.addInterface(esl::com::http::server::requesthandler::Interface::createInterface(
			builtin::http::self::RequestHandler::getImplementation(),
			&builtin::http::self::RequestHandler::createRequestHandler));

	module.addInterface(esl::com::http::server::requesthandler::Interface::createInterface(
			builtin::http::proxy::RequestHandler::getImplementation(),
			&builtin::http::proxy::RequestHandler::createRequestHandler));

	module.addInterface(esl::processing::daemon::Interface::createInterface(
			builtin::daemon::procedures::Daemon::getImplementation(),
			&builtin::daemon::procedures::Daemon::create));

	module.addInterface(esl::processing::procedure::Interface::createInterface(
			builtin::procedure::sleep::Procedure::getImplementation(),
			&builtin::procedure::sleep::Procedure::createProcedure));
}

} /* namespace jerry */
