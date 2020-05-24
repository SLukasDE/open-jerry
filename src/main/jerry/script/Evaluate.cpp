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

#include <jerry/script/Evaluate.h>
#include <jerry/script/Function.h>

#include <memory>
#include <sstream>

namespace jerry {
namespace script {

std::string Evaluate::evaluate(std::string script) {
	while(true) {
		AST ast(std::move(script));

		if(ast.getChildren().empty()) {
			break;
		}

		if(ast.getChildren().size() == 1 && ast.getChildren()[0].get().getType() == AST::text) {
			return ast.getChildren()[0].get().getValue();
		}

		std::stringstream strStream;
		evaluate(strStream, ast);
		script = strStream.str();
	}

	return "";
}

void Evaluate::addFunctionFactory(const FunctionFactory& functionFactory) {
	functionFactories.emplace_back(std::cref(functionFactory));
}

void Evaluate::evaluate(std::ostream& outputStream, const AST& ast) {
	std::vector<std::reference_wrapper<AST>> children = ast.getChildren();

	switch(ast.getType()) {
	case AST::text:
		outputStream << ast.getValue();
		break;
	case AST::list:
		for(const auto& child : children) {
			evaluate(outputStream, child.get());
		}
		break;
	case AST::function:
		if(!children.empty()) {
			std::string functionName;

			if(children[0].get().getType() == AST::text) {
				functionName = children[0].get().getValue();
			}
			else {
				/* 1. make a concatenated string of children[0], whatever it is (list oder function) */
				std::stringstream functionNameStream;
				evaluate(functionNameStream, children[0].get());

				/* 2. the resulting string might contain a formula that needs to be evaluated */
				functionName = evaluate(functionNameStream.str());
			}

			std::vector<std::reference_wrapper<AST>> arguments;
			for(std::size_t i=1; i<children.size(); ++i) {
				arguments.push_back(children[i]);
			}

			outputStream << evaluateFunction(functionName, arguments);
		}
		break;
	}
}

std::string Evaluate::evaluateFunction(const std::string& functionName, std::vector<std::reference_wrapper<AST>> arguments) {
	std::unique_ptr<Function> function;

	/* find matching function  */
	for(auto functionFactory : functionFactories) {
		function = functionFactory.get().create(functionName, arguments.size());
		if(function) {
			break;
		}
	}

	if(!function) {
		throw std::runtime_error("Script error: no matching function \"" + functionName + "\" with " + std::to_string(arguments.size()) + " arguments found.");
	}

	/* set all necessary arguments */
	if(!arguments.empty()) {
		std::vector<bool> argumentsSet(arguments.size(), false);
		std::size_t nextArgument = 0;

		while(nextArgument < arguments.size() && argumentsSet[nextArgument] == false) {
			argumentsSet[nextArgument] = true;
			std::stringstream strStream;
			evaluate(strStream, arguments[nextArgument].get());
			nextArgument = function->setArgument(nextArgument, strStream.str());
		}
	}

	return function->call();
}

} /* namespace script */
} /* namespace jerry */
