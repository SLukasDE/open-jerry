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

#include <openjerry/config/FilePosition.h>

#include <stdexcept>

namespace openjerry {
namespace config {

namespace {
std::string toString(tinyxml2::XMLError xmlError) {
	switch(xmlError) {
	case tinyxml2::XML_SUCCESS:
		return "XML_SUCCESS";
	case tinyxml2::XML_NO_ATTRIBUTE:
		return "XML_NO_ATTRIBUTE";
	case tinyxml2::XML_WRONG_ATTRIBUTE_TYPE:
		return "XML_WRONG_ATTRIBUTE_TYPE";
	case tinyxml2::XML_ERROR_FILE_NOT_FOUND:
		return "XML_ERROR_FILE_NOT_FOUND";
	case tinyxml2::XML_ERROR_FILE_COULD_NOT_BE_OPENED:
		return "XML_ERROR_FILE_COULD_NOT_BE_OPENED";
	case tinyxml2::XML_ERROR_FILE_READ_ERROR:
		return "XML_ERROR_FILE_READ_ERROR";
	case tinyxml2::XML_ERROR_PARSING_ELEMENT:
		return "XML_ERROR_PARSING_ELEMENT";
	case tinyxml2::XML_ERROR_PARSING_ATTRIBUTE:
		return "XML_ERROR_PARSING_ATTRIBUTE";
	case tinyxml2::XML_ERROR_PARSING_TEXT:
		return "XML_ERROR_PARSING_TEXT";
	case tinyxml2::XML_ERROR_PARSING_CDATA:
		return "XML_ERROR_PARSING_CDATA";
	case tinyxml2::XML_ERROR_PARSING_COMMENT:
		return "XML_ERROR_PARSING_COMMENT";
	case tinyxml2::XML_ERROR_PARSING_DECLARATION:
		return "XML_ERROR_PARSING_DECLARATION";
	case tinyxml2::XML_ERROR_PARSING_UNKNOWN:
		return "XML_ERROR_PARSING_UNKNOWN";
	case tinyxml2::XML_ERROR_EMPTY_DOCUMENT:
		return "XML_ERROR_EMPTY_DOCUMENT";
	case tinyxml2::XML_ERROR_MISMATCHED_ELEMENT:
		return "XML_ERROR_MISMATCHED_ELEMENT";
	case tinyxml2::XML_ERROR_PARSING:
		return "XML_ERROR_PARSING";
	case tinyxml2::XML_CAN_NOT_CONVERT_TEXT:
		return "XML_CAN_NOT_CONVERT_TEXT";
	case tinyxml2::XML_NO_TEXT_NODE:
		return "XML_NO_TEXT_NODE";
	case tinyxml2::XML_ELEMENT_DEPTH_EXCEEDED:
		return "XML_ELEMENT_DEPTH_EXCEEDED";
	case tinyxml2::XML_ERROR_COUNT:
		return "XML_ERROR_COUNT";
	default:
		break;
	}

	return "unknown XML error";
}
} /* anonymous namespace */

esl::io::FilePosition::Injector<std::runtime_error> FilePosition::add(const Config& config, const std::string& what) {
	return esl::io::FilePosition::add<std::runtime_error>(config.getFileName(), config.getLineNo(), std::runtime_error(what));
}

esl::io::FilePosition::Injector<std::runtime_error> FilePosition::add(const Config& config, const char* what) {
	return esl::io::FilePosition::add<std::runtime_error>(config.getFileName(), config.getLineNo(), std::runtime_error(what));
}

esl::io::FilePosition::Injector<std::runtime_error> FilePosition::add(const Config& config, tinyxml2::XMLError xmlError) {
	return esl::io::FilePosition::add<std::runtime_error>(config.getFileName(), config.getLineNo(), std::runtime_error(toString(xmlError)));
}

} /* namespace config */
} /* namespace openjerry */
