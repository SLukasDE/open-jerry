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

#ifndef OPENJERRY_CONFIG_FILEPOSITION_H_
#define OPENJERRY_CONFIG_FILEPOSITION_H_

#include <openjerry/config/Config.h>

#include <tinyxml2/tinyxml2.h>

#include <esl/io/FilePosition.h>

#include <string>

namespace openjerry {
namespace config {

class FilePosition final {
public:
	FilePosition() = delete;

	template <class E>
	static esl::io::FilePosition::Injector<E> add(const Config& config, const E& e);

	static esl::io::FilePosition::Injector<std::runtime_error> add(const Config& config, const std::string& what);
	static esl::io::FilePosition::Injector<std::runtime_error> add(const Config& config, const char* what);
	static esl::io::FilePosition::Injector<std::runtime_error> add(const Config& config, tinyxml2::XMLError xmlError);
};

template <class E>
esl::io::FilePosition::Injector<E> FilePosition::add(const Config& config, const E& e) {
	return esl::io::FilePosition::add<E>(config.getFileName(), config.getLineNo(), e);
}

} /* namespace config */
} /* namespace openjerry */

#endif /* OPENJERRY_CONFIG_FILEPOSITION_H_ */
