/*
 * This file is part of Jerry application server.
 * Copyright (C) 2020-2021 Sven Lukas
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

#include <jerry/engine/http/server/RequestContext.h>
#include <jerry/engine/http/server/Context.h>
#include <jerry/engine/http/server/Endpoint.h>
#include <jerry/engine/http/server/Writer.h>
#include <jerry/engine/http/server/ExceptionHandler.h>
#include <jerry/Logger.h>

#include <esl/utility/String.h>

namespace jerry {
namespace engine {
namespace http {
namespace server {

namespace {

Logger logger("jerry::engine::http::server::RequestContext");

const std::string PAGE_301(
		"<!DOCTYPE html>\n"
		"<html>\n"
		"<head>\n"
		"<title>301</title>\n"
		"</head>\n"
		"<body>\n"
		"<h1>301</h1>\n"
		"</body>\n"
		"</html>\n");

std::vector<std::string> makePathList(const std::string& path) {
	return esl::utility::String::split(esl::utility::String::trim(path, '/'), '/');
}

bool checkIfEndingWithSlash(const std::string& path) {
	if(path.empty()) {
		return false;
	}
	return path.at(path.size()-1) == '/';
}
} /* anonymous namespace */

RequestContext::RequestContext(esl::com::http::server::RequestContext& aBaseRequestContext, Writer& aWriter, const Endpoint& aEndpoint)
: baseRequestContext(aBaseRequestContext),
  writer(aWriter),
  connection(*this, aBaseRequestContext.getConnection()),
  context(aEndpoint),
  endpoint(aEndpoint),
  pathList(makePathList(aBaseRequestContext.getRequest().getPath())),
  isEndingWithSlash(checkIfEndingWithSlash(aBaseRequestContext.getRequest().getPath()))
{ }


esl::io::Input RequestContext::createRequestHandler(std::unique_ptr<Writer>& writer, esl::com::http::server::requesthandler::Interface::CreateInput createRequestHandler) {
	ExceptionHandler exceptionHandler;

    /* setting 'exceptionOccured' is important to make hasRequestHandler() return true */
    bool exceptionOccured = exceptionHandler.call(
			[this, createRequestHandler]() { input = createRequestHandler(*this); });

	esl::io::Input rvInput;

    if(exceptionOccured) {
		/* **************** *
		 * Output on logger *
		 * **************** */
    	exceptionHandler.setShowException(true);
    	exceptionHandler.setShowStacktrace(true);

		// TODO
		// This cast is a workaround to avoid a compile time error.
		// Why does the compiler not find the matching dump-method in the base class?
		//exceptionHandler.dump(logger.warn);
    	engine::ExceptionHandler& e(exceptionHandler);
    	e.dump(logger.warn);

		/* ************* *
		* HTTP Response *
		* ************* */
    	exceptionHandler.setShowException(endpoint.get().getShowException());
    	exceptionHandler.setShowStacktrace(endpoint.get().getShowStacktrace());

		exceptionHandler.dump(
				getConnection(),
				[this](unsigned short statusCode) { return endpoint.get().findErrorDocument(statusCode); });

		rvInput = esl::io::Input(std::unique_ptr<esl::io::Writer>(writer.release()));
//		return true;
    }
    else if(input) {
		rvInput = esl::io::Input(std::unique_ptr<esl::io::Writer>(writer.release()));
    }

	return rvInput;
	//return input ? true : false;
}

esl::io::Input& RequestContext::getInput() {
	return input;
}

const esl::io::Input& RequestContext::getInput() const {
	return input;
}

esl::com::http::server::Connection& RequestContext::getConnection() const {
	return connection;
}

const esl::com::http::server::Request& RequestContext::getRequest() const {
	return baseRequestContext.getRequest();
}

void RequestContext::setPath(std::string aPath) {
	path = std::move(aPath);
}

const std::string& RequestContext::getPath() const {
	return path;
}

const std::vector<std::string>& RequestContext::getPathList() {
	return pathList;
}

void RequestContext::setContext(const Context& aContext) {
	context = std::cref(aContext);
}

const Context& RequestContext::getContext() const noexcept {
	return context.get();
}

void RequestContext::setEndpoint(const Endpoint& aEndpoint) {
	std::size_t depth = aEndpoint.getDepth() + aEndpoint.getPathList().size();
	std::string path;

	if(depth < pathList.size()) {
		path += pathList[depth];
	}
	for(std::size_t i = depth+1; i < pathList.size(); ++i) {
		path += "/" + pathList[i];
	}
	if(isEndingWithSlash) {
		path += "/";
	}

	endpoint = std::cref(aEndpoint);

	setPath(path);
}

const Endpoint& RequestContext::getEndpoint() const noexcept {
	return endpoint.get();
}

esl::object::Interface::Object* RequestContext::findObject(const std::string& id) const {
	//esl::object::Interface::Object* object = context.get().findObject(id);
	const esl::object::ObjectContext& objectContext = context.get();
	esl::object::Interface::Object* object = objectContext.findObject<esl::object::Interface::Object>(id);

	if(object == nullptr) {
		object = baseRequestContext.findObject<esl::object::Interface::Object>(id);
	}
	return object;
}

} /* namespace server */
} /* namespace http */
} /* namespace engine */
} /* namespace jerry */
