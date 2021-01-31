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

#include <jerry/script/buildin/FunctionFactory.h>
#include <jerry/script/buildin/And.h>
#include <jerry/script/buildin/Base64.h>
#include <jerry/script/buildin/CompileDate.h>
#include <jerry/script/buildin/Datetime.h>
#include <jerry/script/buildin/Eq.h>
#include <jerry/script/buildin/False.h>
#include <jerry/script/buildin/Iif.h>
#include <jerry/script/buildin/Lt.h>
#include <jerry/script/buildin/Not.h>
#include <jerry/script/buildin/Or.h>
#include <jerry/script/buildin/Substr.h>
#include <jerry/script/buildin/Time.h>
#include <jerry/script/buildin/True.h>

namespace jerry {
namespace script {
namespace buildin {

std::unique_ptr<script::Function> FunctionFactory::create(const std::string& functionName, std::size_t numArguments) const {
	if(functionName == "AND") {
		return std::unique_ptr<script::Function>(new And(numArguments));
	}
	if(functionName == "BASE64" && numArguments == 1) {
		return std::unique_ptr<script::Function>(new Base64);
	}
	if(functionName == "COMPILE_DATE" && numArguments == 0) {
		return std::unique_ptr<script::Function>(new CompileDate);
	}
	if(functionName == "DATETIME" && numArguments == 0) {
		return std::unique_ptr<script::Function>(new Datetime);
	}
	if(functionName == "EQ" && numArguments == 2) {
		return std::unique_ptr<script::Function>(new Eq);
	}
	if(functionName == "FALSE" && numArguments == 0) {
		return std::unique_ptr<script::Function>(new False);
	}
	if(functionName == "IIF" && numArguments == 3) {
		return std::unique_ptr<script::Function>(new Iif);
	}
	if(functionName == "LT" && numArguments == 2) {
		return std::unique_ptr<script::Function>(new Lt);
	}
	if(functionName == "NOT" && numArguments == 1) {
		return std::unique_ptr<script::Function>(new Not);
	}
	if(functionName == "OR") {
		return std::unique_ptr<script::Function>(new Or(numArguments));
	}
	if(functionName == "SUBSTR" && numArguments == 3) {
		return std::unique_ptr<script::Function>(new Substr);
	}
	if(functionName == "TIME" && numArguments == 0) {
		return std::unique_ptr<script::Function>(new Time);
	}
	if(functionName == "TRUE" && numArguments == 0) {
		return std::unique_ptr<script::Function>(new True);
	}
	return nullptr;
}

} /* namespace buildin */
} /* namespace script */
} /* namespace jerry */
