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

#include <jerry/engine/Listener.h>
#include <jerry/engine/Engine.h>
#include <jerry/Module.h>
#include <jerry/Logger.h>
#include <esl/http/server/RequestHandler.h>
#include <esl/http/server/handler/Interface.h>
#include <esl/utility/String.h>
#include <stdexcept>

namespace jerry {
namespace engine {

namespace {
Logger logger("jerry::engine::Listener");
}

Listener::Listener(Engine& aEngine)
: Endpoint(*this, nullptr, ""),
  engine(aEngine)
{
	rootEndpointEntry.endpoint = this;
}

esl::object::parameter::Interface::Object* Listener::getObjectWithEngine(const std::string& id) const {
	logger.trace << "Lookup object with id=\"" + id + "\"\n";

	esl::object::parameter::Interface::Object* object = Context::getObject(id);

	if(object == nullptr) {
		logger.trace << "Object not found in Listener, lookup in engine context.\n";

		object = engine.getObject(id);
	}

	if(object == nullptr) {
		logger.trace << "Object not found in Listener.\n";
	}
	return object;
}

std::unique_ptr<esl::http::server::RequestHandler> Listener::createRequestHandler(esl::http::server::RequestContext& requestContext) {
	std::string path = requestContext.getRequest().getPath();
	bool isEndingWithSlash = false;
	if(!path.empty()) {
		isEndingWithSlash = path.at(path.size()-1) == '/';
	}

	path = esl::utility::String::trim(std::move(path), '/');
	std::vector<std::string> pathList(esl::utility::String::split(path, '/'));

	const Listener::EndpointEntry& endpointEntry = getEndpointEntry(pathList);
	if(endpointEntry.endpoint == nullptr) {
		logger.error << "Internal error: empty endpoint\n";
		return nullptr;
	}

	path.clear();
	for(std::size_t i = endpointEntry.depth; i < pathList.size(); ++i) {
		if(i == endpointEntry.depth) {
			path = pathList[i];
		}
		else {
			path += "/" + pathList[i];
		}
	}
	if(isEndingWithSlash) {
		path += "/";
	}

	return endpointEntry.endpoint->createRequestHandler(requestContext, path);
}

std::vector<std::string> Listener::getEndpointPathList() const {
	return std::vector<std::string>();
}

std::unique_ptr<esl::http::server::RequestHandler> Listener::createRequestHandler(esl::http::server::RequestContext& requestContext, const std::string& path, const Endpoint& endpoint) const {
	return Context::createRequestHandler(requestContext, path, endpoint);
}

const Listener::EndpointEntry& Listener::getEndpointEntry(const std::vector<std::string>& pathList) const {
	const EndpointEntry* result = &rootEndpointEntry;
	const EndpointEntry* currentEndpointEntry = &rootEndpointEntry;

	for(const auto& pathEntry : pathList) {
		auto iter = currentEndpointEntry->entries.find(pathEntry);

		if(iter == std::end(currentEndpointEntry->entries)) {
			break;
		}
		currentEndpointEntry = iter->second.get();

		if(currentEndpointEntry->endpoint) {
			result = currentEndpointEntry;
		}
	}

	return *result;
}

void Listener::registerEndpoint(const Endpoint& endpoint, const std::vector<std::string>& fullPathList) {
	EndpointEntry* endpointEntry = &rootEndpointEntry;

	for(const auto& pathEntry : fullPathList) {
		EndpointEntry* tmpEndpointEntry = endpointEntry->entries[pathEntry].get();
		if(tmpEndpointEntry == nullptr) {
			tmpEndpointEntry = new EndpointEntry;
			tmpEndpointEntry->depth = endpointEntry->depth + 1;
			endpointEntry->entries[pathEntry].reset(tmpEndpointEntry);
		}
		endpointEntry = tmpEndpointEntry;
	}

	if(endpointEntry->endpoint != nullptr) {
		std::string message = "ambiguous definition of endpoint \"";
		for(const auto& pathEntry : fullPathList) {
			message += "/" + pathEntry;
		}
		message += "\"";
		throw std::runtime_error(message);
	}

	endpointEntry->endpoint = &endpoint;
}

} /* namespace engine */
} /* namespace jerry */
