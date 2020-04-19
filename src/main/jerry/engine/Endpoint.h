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

#ifndef JERRY_ENGINE_ENDPOINT_H_
#define JERRY_ENGINE_ENDPOINT_H_

#include <jerry/engine/Context.h>
#include <esl/http/server/RequestContext.h>
#include <string>
#include <vector>
#include <memory>

namespace jerry {
namespace engine {

class Listener;

class Endpoint : public Context {
friend class Context;
public:
	const std::vector<std::string>& getPathList() const;
	std::vector<std::string> getFullPathList() const;
	std::size_t getDepth() const;

	const Endpoint* getParentEndpoint() const;

	void setShowException(bool showException);
	bool getShowException() const;

protected:
	Endpoint(Listener& listener, const Endpoint& parentEndpoint, const Context& parentContext, std::vector<std::string> pathList);

	// only used by Listener
	Endpoint(Listener& listener/*, std::string path*/);

private:
	const Endpoint* parentEndpoint = nullptr;
	std::vector<std::string> pathList;
	std::size_t depth = 0;

	bool showException = false;
};

} /* namespace engine */
} /* namespace jerry */

#endif /* JERRY_ENGINE_ENDPOINT_H_ */
