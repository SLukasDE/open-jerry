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

#ifndef JERRY_ENGINE_BASIC_SERVER_LISTENER_H_
#define JERRY_ENGINE_BASIC_SERVER_LISTENER_H_

#include <jerry/engine/basic/server/Context.h>
#if 0
#include <esl/com/basic/server/RequestContext.h>
#include <esl/io/Input.h>
#include <esl/object/Interface.h>
#endif
#include <string>
#include <vector>

namespace jerry {
namespace engine {
namespace basic {
namespace server {

class Listener : public Context {
public:
	Listener(std::vector<std::string> serverRefIds);

	void dumpTree(std::size_t depth) const override;

	const std::vector<std::string>& getRefIds() const;

	//esl::io::Input createRequestHandler(esl::com::basic::server::RequestContext& requestContext);

private:
	std::vector<std::string> serverRefIds;
};

} /* namespace server */
} /* namespace basic */
} /* namespace engine */
} /* namespace jerry */

#endif /* JERRY_ENGINE_BASIC_SERVER_LISTENER_H_ */
