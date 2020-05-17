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

#include <jerry/http/StatusCode.h>

#include <map>

namespace jerry {
namespace http {

namespace {
std::map<short int, std::string> statusCodeMessages = {
		{100, "Continue"},
		{101, "Switching Protocols"},
		{102, "Processing"},

		{200, "OK"},
		{201, "Created"},
		{202, "Accepted"},
		{203, "Non-Authoritative Information"},
		{204, "No Content"},
		{205, "Reset Content"},
		{206, "Partial Content"},
		{207, "Multi-Status"},
		{208, "Already Reported"},
		{226, "IM Used"},

		{300, "Multiple Choices"},
		{301, "Moved Permanently"},
		{302, "Found (Moved Temporarily)"},
		{303, "See Other"},
		{304, "Not Modified"},
		{305, "Use Proxy"},
		{307, "Temporary Redirect"},
		{308, "Permanent Redirect"},

		{400, "Bad Request"},
		{401, "Authorization Required"},
		{402, "Payment Required"},
		{403, "Forbidden"},
		{404, "Not Found"},
		{405, "Method Not Allowed"},
		{406, "Not Acceptable"},
		{407, "Proxy Authentication Required"},
		{408, "Request Timeout"},
		{409, "Conflict"},
		{410, "Gone"},
		{411, "Length Required"},
		{412, "Precondition Failed"},
		{413, "Request Entity Too Large"},
		{414, "URI Too Long"},
		{415, "Unsupported Media Type"},
		{416, "Requested range not satisfiable"},
		{417, "Requested range not satisfiable"},
		{420, "Policy Not Fulfilled"},
		{421, "Misdirected Request"},
		{422, "Unprocessable Entity"},
		{423, "Locked"},
		{424, "Failed Dependency"},
		{426, "Upgrade Required"},
		{428, "Precondition Required"},
		{429, "Too Many Requests"},
		{431, "Request Header Fields Too Large"},
		{451, "Unavailable For Legal Reasons"},

		{500, "Internal Server Error"},
		{501, "Not Implemented"},
		{502, "Bad Gateway"},
		{503, "Bad Gateway"},
		{504, "Gateway Timeout"},
		{505, "HTTP Version not supported"},
		{506, "Variant Also Negotiates"},
		{507, "Insufficient Storage"},
		{508, "Loop Detected"},
		{509, "Bandwidth Limit Exceeded"},
		{510, "Not Extended"},
		{511, "Network Authentication Required"}
};
}
std::string StatusCode::getMessage(short int statusCode) {
	auto iter = statusCodeMessages.find(statusCode);
	if(iter != std::end(statusCodeMessages)) {
		return iter->second;
	}

	return "";
}

StatusCode::StatusCode() {
	// TODO Auto-generated constructor stub

}

StatusCode::~StatusCode() {
	// TODO Auto-generated destructor stub
}

} /* namespace http */
} /* namespace jerry */
