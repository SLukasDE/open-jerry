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

#ifndef JERRY_CONFIG_HTTP_EXCEPTIONS_H_
#define JERRY_CONFIG_HTTP_EXCEPTIONS_H_

#include <jerry/config/Config.h>
#include <jerry/config/http/ExceptionDocument.h>
#include <jerry/config/OptionalBool.h>
#include <jerry/engine/http/Context.h>

#include <tinyxml2/tinyxml2.h>

#include <string>
#include <vector>
#include <ostream>

namespace jerry {
namespace config {
namespace http {

class Exceptions : public Config {
public:
	Exceptions();
	Exceptions(const std::string& fileName, const tinyxml2::XMLElement& element);

	void save(std::ostream& oStream, std::size_t spaces) const;
	void install(engine::http::Context& engineHttpContext) const;

private:
	bool inheritDocuments = true;
	OptionalBool showExceptions = OptionalBool::obEmpty;
	OptionalBool showStacktrace = OptionalBool::obEmpty;
	std::vector<ExceptionDocument> documents;

	void parseInnerElement(const tinyxml2::XMLElement& element);
};

} /* namespace http */
} /* namespace config */
} /* namespace jerry */

#endif /* JERRY_CONFIG_HTTP_EXCEPTIONS_H_ */
