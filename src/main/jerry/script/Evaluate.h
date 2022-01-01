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

#ifndef JERRY_SCRIPT_EVALUATE_H_
#define JERRY_SCRIPT_EVALUATE_H_

#include <jerry/script/AST.h>
#include <jerry/script/FunctionFactory.h>

#include <ostream>
#include <string>
#include <vector>
#include <functional>

namespace jerry {
namespace script {

class Evaluate {
public:

	std::string evaluate(std::string script);
	void addFunctionFactory(const FunctionFactory& functionFactory);

private:
	void evaluate(std::ostream& outputStream, const AST& ast);
	std::string evaluateFunction(const std::string& functionName, std::vector<std::reference_wrapper<AST>> arguments);

	std::vector<std::reference_wrapper<const FunctionFactory>> functionFactories;
};

} /* namespace script */
} /* namespace jerry */

#endif /* JERRY_SCRIPT_EVALUATE_H_ */
