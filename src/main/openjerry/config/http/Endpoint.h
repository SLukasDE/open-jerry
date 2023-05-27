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

#ifndef OPENJERRY_CONFIG_HTTP_ENDPOINT_H_
#define OPENJERRY_CONFIG_HTTP_ENDPOINT_H_

#include <openjerry/config/Config.h>
#include <openjerry/config/Setting.h>
#include <openjerry/config/http/Entry.h>
#include <openjerry/config/http/Exceptions.h>
#include <openjerry/engine/http/Context.h>

#include <tinyxml2/tinyxml2.h>

#include <string>
#include <vector>
#include <ostream>
#include <memory>

namespace openjerry {
namespace config {
namespace http {

class Endpoint : public Config {
public:
	Endpoint(const Endpoint&) = delete;
	Endpoint(const std::string& fileName, const tinyxml2::XMLElement& element);

	void save(std::ostream& oStream, std::size_t spaces) const;
	void install(engine::http::Context& engineHttpContext) const;

private:
	std::string path;

	bool inherit = true;
	std::vector<Setting> responseHeaders;
	Exceptions exceptions;
	std::vector<std::unique_ptr<Entry>> entries;

	void parseInnerElement(const tinyxml2::XMLElement& element);
};

} /* namespace http */
} /* namespace config */
} /* namespace openjerry */

#endif /* OPENJERRY_CONFIG_HTTP_ENDPOINT_H_ */
