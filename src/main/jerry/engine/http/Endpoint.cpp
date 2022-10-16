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

#include <jerry/engine/http/Endpoint.h>
#include <jerry/Logger.h>

#include <esl/utility/String.h>

namespace jerry {
namespace engine {
namespace http {


namespace {
Logger logger("jerry::engine::http::Endpoint");
} /* anonymous namespace */

Endpoint::Endpoint(ProcessRegistry* processRegistry, const std::string& aPath)
: Context(processRegistry),
  path(esl::utility::String::trim(std::move(aPath), '/'))
//  path("/" + esl::utility::String::trim(std::move(aPath), '/'))
{ }

void Endpoint::dumpTree(std::size_t depth) const {
	for(std::size_t i=0; i<depth; ++i) {
		logger.info << "|   ";
	}
	logger.info << "Path: \"" << getPath() << "\"\n";
	Context::dumpTree(depth);
}

const std::string& Endpoint::getPath() const noexcept {
	return path;
}

const char* Endpoint::getMatch(const std::string& currentPath) const {
	if(currentPath.empty()) {
		return nullptr;
	}

	const char* currentPathCStr = currentPath.c_str();
	std::size_t currentPathSize = currentPath.size();
#if 1
	while(/*currentPathSize > 0 && */currentPathCStr[0] == '/') {
		++currentPathCStr;
		--currentPathSize;

	}
	if(currentPath.find(getPath()) != currentPath.size() - currentPathSize) {
		return nullptr;
	}
#else
	if(currentPathCStr[0] == '/') {
		++currentPathCStr;
		--currentPathSize;

		if(currentPath.find(getPath()) != 1) {
			return nullptr;
		}
	}
	else {
		if(currentPath.find(getPath()) != 0) {
			return nullptr;
		}
	}
#endif
	if(currentPathSize == getPath().size()) {
		return "";
	}
	if(/*currentPath.size() > getPath().size() && */currentPathCStr[getPath().size()] != '/') {
		return nullptr;
	}

	if(currentPathSize == getPath().size() + 1) {
		return &currentPathCStr[getPath().size()];
	}
	return &currentPathCStr[getPath().size() + 1];
}

#if 0
bool Endpoint::isMatch(const std::string& currentPath) const {
	/*
	 * currentPath.rfind(getPath())            == 0
	 * <=>
	 * currentPath.substr(0, getPath().size()) == getPath()
	 */
	return currentPath.rfind(getPath()) == 0 && (currentPath.size() == getPath().size() || currentPath.at(getPath().size()) == '/');
}

std::string Endpoint::getMatchingSubPath(const std::string& currentPath) const {
	if(currentPath.at(getPath().size()) == '/') {
		return currentPath.substr(getPath().size()+1);
	}
	return currentPath.substr(getPath().size());
}
#endif

} /* namespace http */
} /* namespace engine */
} /* namespace jerry */
