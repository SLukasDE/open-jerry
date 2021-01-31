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

#ifndef JERRY_SCRIPT_AST_H_
#define JERRY_SCRIPT_AST_H_

#include <string>
#include <vector>
#include <functional>
#include <memory>

namespace jerry {
namespace script {

class AST {
public:
	enum Type {
		text,
		list,
		function
	};

	AST(std::string script);
	AST() = default;
	~AST() = default;

	Type getType() const noexcept;
	const std::string& getValue() const noexcept;
	std::vector<std::reference_wrapper<AST>> getChildren() const;

	AST& addList();
	AST& addText(std::string text);
	AST& addFunction();

private:
	AST(Type type, std::string text);
	void parse(std::string::const_iterator& iterator, const std::string::const_iterator& end);


	Type type = Type::list;
	std::string value;
	std::vector<std::unique_ptr<AST>> children;
};

} /* namespace script */
} /* namespace jerry */

#endif /* JERRY_SCRIPT_AST_H_ */
