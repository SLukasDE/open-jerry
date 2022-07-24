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

#include <jerry/Plugin.h>

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

#include <eslx/Plugin.h>

#if 1
#include <esl/com/basic/server/RequestHandler.h>
#include <esl/com/http/server/RequestHandler.h>
#include <esl/logging/Appender.h>
#include <esl/logging/Layout.h>
#include <esl/object/Object.h>
#include <esl/processing/Procedure.h>





#else


#include <esl/com/basic/client/ConnectionFactory.h> /* ! */
#include <esl/com/basic/server/RequestHandler.h>
#include <esl/com/basic/server/Socket.h>
#include <esl/com/http/client/ConnectionFactory.h> /* ! */
#include <esl/com/http/server/RequestHandler.h>
#include <esl/com/http/server/Socket.h>
#include <esl/database/ConnectionFactory.h>
#include <esl/logging/Appender.h>
#include <esl/logging/Layout.h>
#include <esl/logging/Logging.h> /* ! */
#include <esl/object/Object.h>
#include <esl/processing/Context.h> /* ! */
#include <esl/processing/Procedure.h>
#include <esl/processing/TaskFactory.h> /* ! */
#include <esl/system/Process.h> /* ! */
#include <esl/system/Signal.h> /* ! */
#include <esl/system/Stacktrace.h> /* ! */

#include <common4esl/logging/MemBufferAppender.h>
#include <common4esl/logging/OStreamAppender.h>
#include <common4esl/logging/DefaultLayout.h>
#include <common4esl/processing/TaskFactory.h>
#include <common4esl/processing/Context.h>

#include <curl4esl/com/http/client/ConnectionFactory.h>

#include <logbook4esl/logging/Logging.h>

#include <mhd4esl/com/http/server/Socket.h>

#include <rdkafka4esl/com/basic/broker/Client.h>
#include <rdkafka4esl/com/basic/client/ConnectionFactory.h>
#include <rdkafka4esl/com/basic/server/Socket.h>

#include <sqlite4esl/database/ConnectionFactory.h>

#include <unixODBC4esl/database/ConnectionFactory.h>

#include <zsystem4esl/system/process/Process.h>
#include <zsystem4esl/system/signal/Signal.h>
#include <zsystem4esl/system/stacktrace/Stacktrace.h>
#endif

#include <memory>

namespace jerry {

void Plugin::install(esl::plugin::Registry& registry, const char* data) {
	esl::plugin::Registry::set(registry);

	eslx::Plugin::install(registry, data);

	/* ***************************** *
	 * builtin basic request handlers *
	 * ***************************** */

	registry.addPlugin<esl::com::basic::server::RequestHandler>(
			"jerry/dump",
			builtin::basic::dump::RequestHandler::createRequestHandler);

	registry.addPlugin<esl::com::basic::server::RequestHandler>(
			"jerry/echo", // builtin::basic::echo::RequestHandler::getImplementation(),
			&builtin::basic::echo::RequestHandler::createRequestHandler);

	registry.addPlugin<esl::com::basic::server::RequestHandler>(
			"jerry/applications", // builtin::basic::application::RequestHandler::getImplementation(),
			&builtin::basic::application::RequestHandler::create);

	/* ***************************** *
	 * builtin http request handlers *
	 * ***************************** */
	registry.addPlugin<esl::com::http::server::RequestHandler>(
			"jerry/applications", // builtin::http::applications::RequestHandler::getImplementation(),
			&builtin::http::applications::RequestHandler::create);

	registry.addPlugin<esl::com::http::server::RequestHandler>(
			"jerry/authentication", // builtin::http::authentication::RequestHandler::getImplementation(),
			&builtin::http::authentication::RequestHandler::createRequestHandler);

	registry.addPlugin<esl::com::http::server::RequestHandler>(
			"jerry/database", // builtin::http::database::RequestHandler::getImplementation(),
			&builtin::http::database::RequestHandler::createRequestHandler);

	registry.addPlugin<esl::com::http::server::RequestHandler>(
			"jerry/dump", // builtin::http::dump::RequestHandler::getImplementation(),
			&builtin::http::dump::RequestHandler::createRequestHandler);

	registry.addPlugin<esl::com::http::server::RequestHandler>(
			"jerry/filebrowser", // builtin::http::filebrowser::RequestHandler::getImplementation(),
			&builtin::http::filebrowser::RequestHandler::createRequestHandler);

	registry.addPlugin<esl::com::http::server::RequestHandler>(
			"jerry/file", // builtin::http::file::RequestHandler::getImplementation(),
			&builtin::http::file::RequestHandler::createRequestHandler);

	registry.addPlugin<esl::com::http::server::RequestHandler>(
			"jerry/log", // builtin::http::file::RequestHandler::getImplementation(),
			&builtin::http::log::RequestHandler::createRequestHandler);

	registry.addPlugin<esl::com::http::server::RequestHandler>(
			"jerry/self", // builtin::http::self::RequestHandler::getImplementation(),
			&builtin::http::self::RequestHandler::createRequestHandler);

	registry.addPlugin<esl::com::http::server::RequestHandler>(
			"jerry/proxy", // builtin::http::proxy::RequestHandler::getImplementation(),
			&builtin::http::proxy::RequestHandler::createRequestHandler);

	/* ****************** *
	 * builtin procedures *
	 * ****************** */
	registry.addPlugin<esl::processing::Procedure>(
			"jerry/authentication-basic-dblookup", // builtin::procedure::authentication::basic::dblookup::Procedure::getImplementation(),
			&builtin::procedure::authentication::basic::dblookup::Procedure::create);

	registry.addPlugin<esl::processing::Procedure>(
			"jerry/authentication-basic-stable", // builtin::procedure::authentication::basic::stable::Procedure::getImplementation(),
			&builtin::procedure::authentication::basic::stable::Procedure::create);

	registry.addPlugin<esl::processing::Procedure>(
			"jerry/authentication-jwt", // builtin::procedure::authentication::jwt::Procedure::getImplementation(),
			&builtin::procedure::authentication::jwt::Procedure::create);

	registry.addPlugin<esl::processing::Procedure>(
			"jerry/authorization-cache", // builtin::procedure::authorization::cache::Procedure::getImplementation(),
			&builtin::procedure::authorization::cache::Procedure::create);

	registry.addPlugin<esl::processing::Procedure>(
			"jerry/authorization-dblookup", // builtin::procedure::authorization::dblookup::Procedure::getImplementation(),
			&builtin::procedure::authorization::dblookup::Procedure::create);

	registry.addPlugin<esl::processing::Procedure>(
			"jerry/authorization-jwt", // builtin::procedure::authorization::jwt::Procedure::getImplementation(),
			&builtin::procedure::authorization::jwt::Procedure::create);

	registry.addPlugin<esl::processing::Procedure>(
			"jerry/detach", // builtin::procedure::list::Procedure::getImplementation(),
			&builtin::procedure::detach::Procedure::create);

	registry.addPlugin<esl::processing::Procedure>(
			"jerry/list", // builtin::procedure::list::Procedure::getImplementation(),
			&builtin::procedure::list::Procedure::create);

	registry.addPlugin<esl::processing::Procedure>(
			"jerry/return-code", // builtin::procedure::returncode::Procedure::getImplementation(),
			&builtin::procedure::returncode::Procedure::create);

	registry.addPlugin<esl::processing::Procedure>(
			"jerry/sleep", // builtin::procedure::sleep::Procedure::getImplementation(),
			&builtin::procedure::sleep::Procedure::create);

	registry.addPlugin<esl::processing::Procedure>(
			"jerry", // Procedure::getImplementation(),
			&engine::main::Context::create);

	/* *************** *
	 * builtin objects *
	 * *************** */
	registry.addPlugin<esl::object::Object>(
			"std/int",
			&builtin::object::standard::Int::create);

	registry.addPlugin<esl::object::Object>(
			"std/map<string,string>",
			&builtin::object::standard::MapStringString::create);

	registry.addPlugin<esl::object::Object>(
			"std/set<int>",
			&builtin::object::standard::SetInt::create);

	registry.addPlugin<esl::object::Object>(
			"std/set<string>",
			&builtin::object::standard::SetString::create);

	registry.addPlugin<esl::object::Object>(
			"std/string",
			&builtin::object::standard::String::create);

	registry.addPlugin<esl::object::Object>(
			"std/vector<int>",
			&builtin::object::standard::VectorInt::create);

	registry.addPlugin<esl::object::Object>(
			"std/vector<pair<string,string>>",
			&builtin::object::standard::VectorPairStringString::create);

	registry.addPlugin<esl::object::Object>(
			"std/vector<string>",
			&builtin::object::standard::VectorString::create);

	registry.addPlugin<esl::object::Object>(
			"jerry/applications",
			&builtin::object::applications::Object::create);



	/* **** *
	 * eslx *
	 * **** */
#if 0
#if 1
	// logbook4esl
	registry.addPlugin<esl::logging::Logging>(
			"eslx/logging/Logging",
			&logbook4esl::logging::Logging::create);


	//boostst4esl::Plugin::install(registry, data);



	// rdkafka4esl
	registry.addPlugin<esl::object::Object>(
			"eslx/com/basic/broker/KafkaClient",
			&rdkafka4esl::com::basic::broker::Client::create);

	registry.addPlugin<esl::com::basic::client::ConnectionFactory>(
			"eslx/com/basic/client/KafkaConnectionFactory",
			&rdkafka4esl::com::basic::client::ConnectionFactory::create);

	registry.addPlugin<esl::com::basic::server::Socket>(
			"eslx/com/basic/server/KafkaSocket",
			&rdkafka4esl::com::basic::server::Socket::create);



	// mhd4esl
	registry.addPlugin<esl::com::http::server::Socket>(
			"eslx/com/http/server/Socket",
			&mhd4esl::com::http::server::Socket::create);



	// curl4esl
	registry.addPlugin<esl::com::http::client::ConnectionFactory>(
			"eslx/com/http/client/ConnectionFactory",
			curl4esl::com::http::client::ConnectionFactory::create);



	// unixODBC4esl
	registry.addPlugin<esl::database::ConnectionFactory>(
			"eslx/database/ODBCConnectionFactory",
			&unixODBC4esl::database::ConnectionFactory::create);



	// sqlite4esl
	registry.addPlugin<esl::object::Object>(
			"eslx/database/SQLiteConnectionFactory",
			&sqlite4esl::database::ConnectionFactory::createObject);

	registry.addPlugin<esl::database::ConnectionFactory>(
			"eslx/database/SQLiteConnectionFactory",
			&sqlite4esl::database::ConnectionFactory::createConnectionFactory);



	// common4esl
	registry.addPlugin<esl::logging::Appender>(
			"eslx/logging/MemBufferAppender",
			&common4esl::logging::MemBufferAppender::create);

	registry.addPlugin<esl::logging::Appender>(
			"eslx/logging/OStreamAppender",
			&common4esl::logging::OStreamAppender::create);

	registry.addPlugin<esl::logging::Layout>(
			"eslx/logging/DefaultLayout",
			&common4esl::logging::DefaultLayout::create);

	registry.addPlugin<esl::processing::Context>(
			"eslx/processing/Context",
			&common4esl::processing::Context::create);

	registry.addPlugin<esl::processing::TaskFactory>(
			"eslx/processing/TaskFactory",
			&common4esl::processing::TaskFactory::create);


	// zsystem4esl
	registry.addPlugin<esl::system::Process>(
			"eslx/system/Process",
			&zsystem4esl::system::process::Process::create);

	registry.addPlugin<esl::system::Signal>(
			"eslx/system/Signal",
			&zsystem4esl::system::signal::Signal::create);

	registry.addPlugin<esl::system::Stacktrace>(
			"eslx/system/Stacktrace",
			&zsystem4esl::system::stacktrace::Stacktrace::create);
#else
	/* ************ *
	 * esl::logging *
	 * ************ */
	/* *********************** *
	 * builtin logging layouts *
	 * *********************** */
	registry.copyPlugin<esl::logging::Layout>("eslx/logging/DefaultLayout", "jerry/default");

	/* ************************* *
	 * builtin logging appenders *
	 * ************************* */
	registry.copyPlugin<esl::logging::Appender>("eslx/logging/MemBufferAppender", "jerry/membuffer");
	registry.copyPlugin<esl::logging::Appender>("eslx/logging/OStreamAppender", "jerry/ostream");
#endif
#endif

}

} /* namespace jerry */
