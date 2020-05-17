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

#include <jerry/engine/Endpoint.h>
#include <jerry/engine/Listener.h>
#include <jerry/engine/Engine.h>
#include <jerry/Logger.h>
#include <esl/utility/String.h>
//#include <stdexcept>

namespace jerry {
namespace engine {

namespace {
Logger logger("jerry::engine::Endpoint");
} /* anonymous namespace */

Endpoint::Endpoint(Listener& listener, const Endpoint& aParentEndpoint, const Context& parentContext, std::vector<std::string> aPathList)
: Context(listener, *this, parentContext),
  parentEndpoint(&aParentEndpoint),
  pathList(std::move(aPathList)),
  depth(parentEndpoint->getDepth() + parentEndpoint->getPathList().size())
{
	listener.registerEndpoint(*this);
}

// only used by Listener
Endpoint::Endpoint(Listener& listener/*, std::string path*/)
: Context(listener),
  depth(0)
{ }

const std::vector<std::string>& Endpoint::getPathList() const {
	return pathList;
}

std::vector<std::string> Endpoint::getFullPathList() const {
	std::vector<std::string> result;

	if(getParentEndpoint()) {
		result = getParentEndpoint()->getFullPathList();
	}
	result.insert(std::end(result), std::begin(pathList), std::end(pathList));

	return result;
}

std::size_t Endpoint::getDepth() const {
	return depth;
}

const Endpoint* Endpoint::getParentEndpoint() const {
	return parentEndpoint;
}

void Endpoint::setShowException(bool aShowException) {
	showException = aShowException ? obTrue : obFalse;
}

bool Endpoint::getShowException() const {
	switch(showException) {
	case obTrue:
		return true;
	case obFalse:
		return false;
	default:
		break;
	}

	if(getParentEndpoint()) {
		return getParentEndpoint()->getShowException();
	}

	return true;
}

void Endpoint::setShowStacktrace(bool aShowStacktrace) {
	showStacktrace = aShowStacktrace ? obTrue : obFalse;
}

bool Endpoint::getShowStacktrace() const {
	switch(showStacktrace) {
	case obTrue:
		return true;
	case obFalse:
		return false;
	default:
		break;
	}

	if(getParentEndpoint()) {
		return getParentEndpoint()->getShowStacktrace();
	}

	return false;
}

void Endpoint::setInheritErrorDocuments(bool aInheritErrorDocuments) {
	inheritErrorDocuments = aInheritErrorDocuments;
}

bool Endpoint::getInheritErrorDocuments() const {
	return inheritErrorDocuments;
}

void Endpoint::addErrorDocument(unsigned short statusCode, const std::string& path, bool parse) {
	errorDocuments.insert(std::make_pair(statusCode, std::make_pair(path, parse)));
}

const std::pair<std::string, bool>* Endpoint::findErrorDocument(unsigned short statusCode) const {
	const auto iter = errorDocuments.find(statusCode);

	if(iter == std::end(errorDocuments)) {
		if(getInheritErrorDocuments() && getParentEndpoint()) {
			return getParentEndpoint()->findErrorDocument(statusCode);
		}
		return nullptr;
	}

	return &iter->second;
}

void Endpoint::addHeader(std::string key, std::string value) {
	headers.insert(std::make_pair(key, value));
}

const std::map<std::string, std::string>& Endpoint::getHeaders() const {
	return headers;
}

} /* namespace engine */
} /* namespace jerry */
