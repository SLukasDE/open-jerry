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

#include <jerry/config/Config.h>
#include <jerry/config/FilePosition.h>
#include <jerry/Logger.h>

#include <cstdlib>

namespace jerry {
namespace config {

namespace {
Logger logger("jerry::config::Config");
} /* anonymous namespace */

Config::Config(const std::string& aFileName)
: fileName(aFileName),
  lineNo(-1)
{ }

Config::Config(const std::string& aFileName, const tinyxml2::XMLElement& aElement)
: fileName(aFileName),
  lineNo(aElement.GetLineNum())
{ }

std::string Config::evaluate(const std::string& expression, const std::string& language) const {
	if(language == "plain") {
		return expression;
	}

	std::string value;
	std::string var;
	enum {
		intro,
		begin,
		end
	} state = end;

	for(std::size_t i=0; i<expression.size(); ++i) {
		if(state == begin) {
			if(expression.at(i) == '}') {
				char* val = getenv(var.c_str());
				if(val == nullptr) {
					throw FilePosition::add(*this, "No value available for variable \"" + var + "\" in expression: \"" + expression + "\"");
				}
				value += val;
				state = end;
				var.clear();
			}
			else {
				var += expression.at(i);
			}
		}
		else if(state == intro) {
			if(expression.at(i) == '{') {
				state = begin;
			}
			else {
				throw FilePosition::add(*this, "Syntax error in expression: \"" + expression + "\"");
			}
		}
		else {
			if(expression.at(i) == '$') {
				state = intro;
			}
			else {
				value += expression.at(i);
			}
		}
	}

	return value;
}

const std::string& Config::getFileName() const noexcept {
	return fileName;
}

int Config::getLineNo() const noexcept {
	return lineNo;
}

std::pair<std::string, int> Config::getXMLFile() const noexcept {
	return std::pair<std::string, int>(fileName, lineNo);
}

std::pair<std::string, int> Config::setXMLFile(const std::string& aFileName, int aLineNo) {
	std::pair<std::string, int> oldXmlFile(fileName, lineNo);

	fileName = aFileName;
	lineNo = aLineNo;

	return oldXmlFile;
}

std::pair<std::string, int> Config::setXMLFile(const std::string& aFileName, const tinyxml2::XMLElement& aElement) {
	std::pair<std::string, int> oldXmlFile(fileName, lineNo);

	fileName = aFileName;
	lineNo = aElement.GetLineNum();

	return oldXmlFile;
}

std::pair<std::string, int> Config::setXMLFile(const std::pair<std::string, int>& aXmlFile) {
	std::pair<std::string, int> oldXmlFile(fileName, lineNo);

	fileName = aXmlFile.first;
	lineNo = aXmlFile.second;

	return oldXmlFile;
}

std::string Config::makeSpaces(std::size_t spaces) const {
	std::string rv;
	for(std::size_t i=0; i<spaces; ++i) {
		rv += " ";
	}
	return rv;
}

bool Config::stringToBool(bool& b, std::string str) {
	if(str == "true") {
		b = true;
	}
	else if(str == "false") {
		b = false;
	}
	else {
		return false;
	}
	return true;
}

} /* namespace config */
} /* namespace jerry */
