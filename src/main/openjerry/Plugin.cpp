#include <openjerry/Plugin.h>
#include <openjerry/builtin/http/dump/RequestHandler.h>
#include <openjerry/builtin/http/file/RequestHandler.h>
#include <openjerry/builtin/http/filebrowser/RequestHandler.h>
#include <openjerry/builtin/http/log/RequestHandler.h>
#include <openjerry/builtin/http/self/RequestHandler.h>

namespace openjerry {

void Plugin::install(esl::plugin::Registry& registry, const char* data) {
	esl::plugin::Registry::set(registry);

	registry.addPlugin("jerry/dump", openjerry::builtin::http::dump::RequestHandler::createRequestHandler);
	registry.addPlugin("jerry/file", openjerry::builtin::http::file::RequestHandler::createRequestHandler);
	registry.addPlugin("jerry/filebrowser", openjerry::builtin::http::filebrowser::RequestHandler::createRequestHandler);
	registry.addPlugin("jerry/log", openjerry::builtin::http::log::RequestHandler::createRequestHandler);
	registry.addPlugin("jerry/self", openjerry::builtin::http::self::RequestHandler::createRequestHandler);
}

} /* namespace openjerry */
