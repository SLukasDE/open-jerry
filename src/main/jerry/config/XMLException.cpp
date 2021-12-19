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

#include <jerry/config/XMLException.h>

namespace jerry {
namespace config {

namespace {
std::string createMessage(tinyxml2::XMLError xmlError) {
	switch(xmlError) {
	case tinyxml2::XML_SUCCESS:
		return "XML error: other (XML_SUCCESS)";
	case tinyxml2::XML_NO_ATTRIBUTE:
		return "XML error: XML_NO_ATTRIBUTE";
	case tinyxml2::XML_WRONG_ATTRIBUTE_TYPE:
		return "XML error: XML_WRONG_ATTRIBUTE_TYPE";
	case tinyxml2::XML_ERROR_FILE_NOT_FOUND:
		return "XML error: XML_ERROR_FILE_NOT_FOUND";
	case tinyxml2::XML_ERROR_FILE_COULD_NOT_BE_OPENED:
		return "XML error: XML_ERROR_FILE_COULD_NOT_BE_OPENED";
	case tinyxml2::XML_ERROR_FILE_READ_ERROR:
		return "XML error: XML_ERROR_FILE_READ_ERROR";
	case tinyxml2::XML_ERROR_PARSING_ELEMENT:
		return "XML error: XML_ERROR_PARSING_ELEMENT";
	case tinyxml2::XML_ERROR_PARSING_ATTRIBUTE:
		return "XML error: XML_ERROR_PARSING_ATTRIBUTE";
	case tinyxml2::XML_ERROR_PARSING_TEXT:
		return "XML error: XML_ERROR_PARSING_TEXT";
	case tinyxml2::XML_ERROR_PARSING_CDATA:
		return "XML error: XML_ERROR_PARSING_CDATA";
	case tinyxml2::XML_ERROR_PARSING_COMMENT:
		return "XML error: XML_ERROR_PARSING_COMMENT";
	case tinyxml2::XML_ERROR_PARSING_DECLARATION:
		return "XML error: XML_ERROR_PARSING_DECLARATION";
	case tinyxml2::XML_ERROR_PARSING_UNKNOWN:
		return "XML error: XML_ERROR_PARSING_UNKNOWN";
	case tinyxml2::XML_ERROR_EMPTY_DOCUMENT:
		return "XML error: XML_ERROR_EMPTY_DOCUMENT";
	case tinyxml2::XML_ERROR_MISMATCHED_ELEMENT:
		return "XML error: XML_ERROR_MISMATCHED_ELEMENT";
	case tinyxml2::XML_ERROR_PARSING:
		return "XML error: XML_ERROR_PARSING";
	case tinyxml2::XML_CAN_NOT_CONVERT_TEXT:
		return "XML error: XML_CAN_NOT_CONVERT_TEXT";
	case tinyxml2::XML_NO_TEXT_NODE:
		return "XML error: XML_NO_TEXT_NODE";
	case tinyxml2::XML_ELEMENT_DEPTH_EXCEEDED:
		return "XML error: XML_ELEMENT_DEPTH_EXCEEDED";
	case tinyxml2::XML_ERROR_COUNT:
		return "XML error: XML_ERROR_COUNT";
	default:
		break;
	}

	return "XML error: other";
}
}

XMLException::XMLException(const std::string& fileName, int lineNo, tinyxml2::XMLError xmlError)
: fullMessage("file \"" + fileName + "\", line " + std::to_string(lineNo) + ": " + createMessage(xmlError))
{ }

XMLException::XMLException(const std::string& fileName, int lineNo, const std::string& message)
: fullMessage("file \"" + fileName + "\", line " + std::to_string(lineNo) + ": " + message)
{ }

XMLException::XMLException(const Config& config, tinyxml2::XMLError xmlError)
: fullMessage("file \"" + config.getFileName() + "\", line " + std::to_string(config.getLineNo()) + ": " + createMessage(xmlError))
{ }

XMLException::XMLException(const Config& config, const std::string& message)
: fullMessage("file \"" + config.getFileName() + "\", line " + std::to_string(config.getLineNo()) + ": " + message)
{ }

const char* XMLException::what() const noexcept {
	return fullMessage.c_str();
}

} /* namespace config */
} /* namespace jerry */
