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

#include <jerry/engine/http/Endpoint.h>
#include <jerry/engine/http/Listener.h>
#include <jerry/engine/Engine.h>
#include <jerry/Logger.h>

#include <esl/utility/String.h>
#include <esl/Stacktrace.h>

#include <stdexcept>

namespace jerry {
namespace engine {
namespace http {

namespace {
Logger logger("jerry::engine::http::Endpoint");
} /* anonymous namespace */

Endpoint::Endpoint(Listener& listener, const Endpoint& aParentEndpoint, const Context& parentContext, std::vector<std::string> aPathList, bool inheritObjects)
: Context(listener, *this, &parentContext, inheritObjects),
  parentEndpoint(&aParentEndpoint),
  pathList(std::move(aPathList)),
  depth(parentEndpoint->getDepth() + parentEndpoint->getPathList().size())
{
	listener.registerEndpoint(*this);
}

// only used by Listener
Endpoint::Endpoint(Listener& listener, bool inheritObjects)
: Context(listener, listener, nullptr, inheritObjects),
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

void Endpoint::setShowException(bool aValue) {
	OptionalBool value = aValue ? obTrue : obFalse;

	if(showException != obEmpty) {
		if(showException != value) {
	        throw std::runtime_error("Ambiguous definition of enabling exception output.");
		}
        logger.warn << "Multiple definition of enabling exception output.\n";
	}

	showException = value;
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

void Endpoint::setShowStacktrace(bool aValue) {
	OptionalBool value = aValue ? obTrue : obFalse;

	if(showStacktrace != obEmpty) {
		if(showStacktrace != value) {
	        throw std::runtime_error("Ambiguous definition of enabling stacktrace output.");
		}
        logger.warn << "Multiple definition of enabling stacktrace output.\n";
	}

	showStacktrace = value;
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
	if(errorDocuments.count(statusCode) > 0) {
        throw std::runtime_error("There are multiple error documents defined for http status code \"" + std::to_string(statusCode) + "\".");
	}

	Document document(path);
	document.setLanguage(parse ? Document::builtinScript : Document::none);
	errorDocuments.insert(std::make_pair(statusCode, document));
}

const Document* Endpoint::findErrorDocument(unsigned short statusCode) const {
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

void Endpoint::dumpTree(std::size_t depth) const {
	for(std::size_t i=0; i<depth; ++i) {
		logger.info << "|   ";
	}
	logger.info << "Path: \"";
	for(const auto& path : getPathList()) {
		logger.info << "/" << path;
	}
	logger.info << "\"\n";

	if(showException != obEmpty) {
		for(std::size_t i=0; i<depth; ++i) {
			logger.info << "|   ";
		}
		logger.info << "showException: ";
		switch(showException) {
		case obTrue:
			logger.info << "true\n";
			break;
		case obFalse:
			logger.info << "false\n";
			break;
		default:
			break;
		}
		logger.info << "\"\n";
	}

	if(showStacktrace != obEmpty) {
		for(std::size_t i=0; i<depth; ++i) {
			logger.info << "|   ";
		}
		logger.info << "showStacktrace: ";
		switch(showStacktrace) {
		case obTrue:
			logger.info << "true\n";
			break;
		case obFalse:
			logger.info << "false\n";
			break;
		default:
			break;
		}
		logger.info << "\"\n";
	}

	for(std::size_t i=0; i<depth; ++i) {
		logger.info << "|   ";
	}
	logger.info << "inheritErrorDocuments: ";
	if(inheritErrorDocuments) {
		logger.info << "true\n";
	}
	else {
		logger.info << "false\n";
	}

	if(errorDocuments.empty() == false) {
		for(std::size_t i=0; i<depth; ++i) {
			logger.info << "|   ";
		}
		logger.info << "Documents for Status-Code:\n";
		for(const auto& statusCodeDocument : errorDocuments) {
			for(std::size_t i=0; i<depth; ++i) {
				logger.info << "|   ";
			}
			logger.info << "  " << statusCodeDocument.first << "\n";
		}
	}

	if(headers.empty() == false) {
		for(std::size_t i=0; i<depth; ++i) {
			logger.info << "|   ";
		}
		logger.info << "Additional response headers:\n";
		for(const auto& header : headers) {
			for(std::size_t i=0; i<depth; ++i) {
				logger.info << "|   ";
			}
			logger.info << "  \"" << header.first << "\"=\"" << header.second << "\"\n";
		}
	}

	Context::dumpTree(depth);
}

} /* namespace http */
} /* namespace engine */
} /* namespace jerry */
