#include <openjerry/Plugin.h>
#include <openjerry/builtin/http/authentication/RequestHandler.h>
#include <openjerry/builtin/http/dump/RequestHandler.h>
#include <openjerry/builtin/http/file/RequestHandler.h>
#include <openjerry/builtin/http/filebrowser/RequestHandler.h>
#include <openjerry/builtin/http/log/RequestHandler.h>
#include <openjerry/builtin/http/self/RequestHandler.h>
#include <openjerry/builtin/procedure/authentication/basic/dblookup/Procedure.h>
#include <openjerry/builtin/procedure/authentication/basic/stable/Procedure.h>
#include <openjerry/builtin/procedure/authentication/jwt/Procedure.h>
#include <openjerry/builtin/procedure/authorization/cache/Procedure.h>
#include <openjerry/builtin/procedure/authorization/dblookup/Procedure.h>
#include <openjerry/builtin/procedure/authorization/jwt/Procedure.h>
#include <openjerry/builtin/procedure/sleep/Procedure.h>

namespace openjerry {

void Plugin::install(esl::plugin::Registry& registry, const char* data) {
	esl::plugin::Registry::set(registry);

	registry.addPlugin("jerry/authentication", openjerry::builtin::http::authentication::RequestHandler::createRequestHandler);
	registry.addPlugin("jerry/dump",           openjerry::builtin::http::dump::RequestHandler::createRequestHandler);
	registry.addPlugin("jerry/file",           openjerry::builtin::http::file::RequestHandler::createRequestHandler);
	registry.addPlugin("jerry/filebrowser",    openjerry::builtin::http::filebrowser::RequestHandler::createRequestHandler);
	registry.addPlugin("jerry/log",            openjerry::builtin::http::log::RequestHandler::createRequestHandler);
	registry.addPlugin("jerry/self",           openjerry::builtin::http::self::RequestHandler::createRequestHandler);

	registry.addPlugin("jerry/authentication-basic-dblookup", openjerry::builtin::procedure::authentication::basic::dblookup::Procedure::create);
	registry.addPlugin("jerry/authentication-basic-stable",   openjerry::builtin::procedure::authentication::basic::stable::Procedure::create);
	registry.addPlugin("jerry/authentication-jwt",            openjerry::builtin::procedure::authentication::jwt::Procedure::create);
	registry.addPlugin("jerry/authorization-cache",           openjerry::builtin::procedure::authorization::cache::Procedure::create);
	registry.addPlugin("jerry/authorization-dblookup",        openjerry::builtin::procedure::authorization::dblookup::Procedure::create);
	registry.addPlugin("jerry/authorization-jwt",             openjerry::builtin::procedure::authorization::jwt::Procedure::create);
	
	registry.addPlugin("jerry/sleep",        openjerry::builtin::procedure::sleep::Procedure::create);
}

} /* namespace openjerry */
