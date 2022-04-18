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

#include <jerry/config/logging/LevelSetting.h>
#include <jerry/config/XMLException.h>

#include <esl/utility/String.h>

namespace jerry {
namespace config {
namespace logging {

LevelSetting::LevelSetting(const std::string& fileName, const tinyxml2::XMLElement& element)
: Config(fileName, element)
{
	if(element.GetUserData() != nullptr) {
		throw XMLException(*this, "Element has user data but it should be empty");
	}

	bool levelDefined = false;

	for(const tinyxml2::XMLAttribute* attribute = element.FirstAttribute(); attribute != nullptr; attribute = attribute->Next()) {
		if(std::string(attribute->Name()) == "scope") {
			if(scope != "") {
				throw XMLException(*this, "Multiple definition of attribute 'scope'");
			}
			scope = attribute->Value();
			if(scope == "") {
				throw XMLException(*this, "Value \"\" of attribute 'scope' is invalid.");
			}
		}
		else if(std::string(attribute->Name()) == "level") {
			if(levelDefined) {
				throw XMLException(*this, "Multiple definition of attribute 'level'");
			}
			std::string levelStr = esl::utility::String::toUpper(attribute->Value());

			if(levelStr == "SILENT") {
				level = esl::logging::Level::SILENT;
			}
			else if(levelStr == "ERROR") {
				level = esl::logging::Level::ERROR;
			}
			else if(levelStr == "WARN") {
				level = esl::logging::Level::WARN;
			}
			else if(levelStr == "INFO") {
				level = esl::logging::Level::INFO;
			}
			else if(levelStr == "DEBUG") {
				level = esl::logging::Level::DEBUG;
			}
			else if(levelStr == "TRACE") {
				level = esl::logging::Level::TRACE;
			}
			else {
				throw XMLException(*this, "Value \"" + levelStr + "\" of attribute 'level' is invalid. "
						"Valid values are \"SILENT\", \"ERROR\", \"WARN\", \"INFO\", \"DEBUG\" and \"TRACE\"");
			}
			levelDefined = true;
		}
		else {
			throw XMLException(*this, "Unknown attribute '" + std::string(attribute->Name()) + "'");
		}
	}

	if(scope == "") {
		throw XMLException(*this, "Missing attribute 'scope'");
	}
	if(!levelDefined) {
		throw XMLException(*this, "Missing attribute 'level'");
	}
}

esl::logging::Level LevelSetting::getLevel() const {
	return level;
}

const std::string& LevelSetting::getScope() const {
	return scope;
}

void LevelSetting::save(std::ostream& oStream, std::size_t spaces) const {
	switch(level) {
	case esl::logging::Level::SILENT:
		oStream << makeSpaces(spaces) << "<setting scope=\"" << scope << "\" level=\"silent\"/>\n";
		break;
	case esl::logging::Level::ERROR:
		oStream << makeSpaces(spaces) << "<setting scope=\"" << scope << "\" level=\"error\"/>\n";
		break;
	case esl::logging::Level::WARN:
		oStream << makeSpaces(spaces) << "<setting scope=\"" << scope << "\" level=\"warn\"/>\n";
		break;
	case esl::logging::Level::INFO:
		oStream << makeSpaces(spaces) << "<setting scope=\"" << scope << "\" level=\"info\"/>\n";
		break;
	case esl::logging::Level::DEBUG:
		oStream << makeSpaces(spaces) << "<setting scope=\"" << scope << "\" level=\"debug\"/>\n";
		break;
	case esl::logging::Level::TRACE:
		oStream << makeSpaces(spaces) << "<setting scope=\"" << scope << "\" level=\"trace\"/>\n";
		break;
	}
}

} /* namespace logging */
} /* namespace config */
} /* namespace jerry */
