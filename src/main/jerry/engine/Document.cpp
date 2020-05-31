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

#include <jerry/engine/Document.h>

namespace jerry {
namespace engine {

Document::Document(std::string aPath)
: path(std::move(aPath))
{ }

const std::string& Document::getPath() const noexcept {
	return path;
}

void Document::setLanguage(Language language) {
	switch(language) {
	case builtinScript:
		setLanguage("builtinScript");
		break;
	default:
		setLanguage("");
		break;
	}
}

void Document::setLanguage(std::string aLanguage) {
	language = std::move(aLanguage);
}

const std::string& Document::getLanguage() const noexcept {
	return language;
}

} /* namespace engine */
} /* namespace jerry */
