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

#ifndef JERRY_CONFIG_HTTP_HTTPCONTEXT_H_
#define JERRY_CONFIG_HTTP_HTTPCONTEXT_H_

#include <jerry/config/Config.h>
#include <jerry/config/Setting.h>
#include <jerry/config/http/Entry.h>
#include <jerry/config/http/Exceptions.h>
#include <jerry/engine/Engine.h>

#include <tinyxml2/tinyxml2.h>

#include <vector>
#include <ostream>
#include <string>
#include <memory>

namespace jerry {
namespace config {
namespace http {

class HttpContext : public Config {
public:
	HttpContext(const HttpContext&) = delete;
	HttpContext(const std::string& fileName, const tinyxml2::XMLElement& element);

	void save(std::ostream& oStream, std::size_t spaces) const;
	void install(engine::Engine& engine) const;

private:
	std::string id;

	bool inherit = true;
	std::vector<Setting> responseHeaders;
	http::Exceptions exceptions;
	std::vector<std::unique_ptr<Entry>> entries;

	void parseInnerElement(const tinyxml2::XMLElement& element);
};

} /* namespace http */
} /* namespace config */
} /* namespace jerry */

#endif /* JERRY_CONFIG_HTTP_HTTPCONTEXT_H_ */
