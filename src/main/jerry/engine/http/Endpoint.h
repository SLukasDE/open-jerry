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

#ifndef JERRY_ENGINE_HTTP_ENDPOINT_H_
#define JERRY_ENGINE_HTTP_ENDPOINT_H_

#include <jerry/engine/http/Context.h>
#include <jerry/engine/ProcessRegistry.h>

#include <string>

namespace jerry {
namespace engine {
namespace http {

class Endpoint : public Context {
public:
	Endpoint(ProcessRegistry* processRegistry, const std::string& path);

	void dumpTree(std::size_t depth) const override;

	const std::string& getPath() const noexcept;
	bool isMatch(const std::string& currentPath) const;

private:
	std::string path;
};


} /* namespace http */
} /* namespace engine */
} /* namespace jerry */

#endif /* JERRY_ENGINE_HTTP_ENDPOINT_H_ */
