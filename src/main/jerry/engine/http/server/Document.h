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

#ifndef JERRY_ENGINE_HTTP_SERVER_DOCUMENT_H_
#define JERRY_ENGINE_HTTP_SERVER_DOCUMENT_H_

#include <string>

namespace jerry {
namespace engine {
namespace http {
namespace server {

class Document {
public:
	enum Language {
		none,
		builtinScript
	};

	Document(std::string path);

	const std::string& getPath() const noexcept;

	void setLanguage(Language language);
	void setLanguage(std::string language);
	const std::string& getLanguage() const noexcept;

private:
	std::string path;
	std::string language;
};

} /* namespace server */
} /* namespace http */
} /* namespace engine */
} /* namespace jerry */

#endif /* JERRY_ENGINE_HTTP_SERVER_DOCUMENT_H_ */
