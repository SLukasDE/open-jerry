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

#include <jerry/cgi/client/consumer/ResponseHandler.h>
#if 0
#include <jerry/Logger.h>

#include <esl/utility/String.h>

#include <vector>

namespace jerry {
namespace cgi {
namespace client {
namespace consumer {
namespace {
Logger logger("jerry::cgi::client::consumer::ResponseHandler");
}

ResponseHandler::ResponseHandler(esl::http::client::ResponseHandler* aResponseHandler)
: responseHandler(aResponseHandler)
{ }

std::size_t ResponseHandler::read(esl::system::Interface::FileDescriptor& fileDescriptor) {
	/* read content from the CGI script */
	char tmpBuffer[4096];
	std::size_t count = fileDescriptor.read(tmpBuffer, sizeof(tmpBuffer));

	if(count != 0) {
		if(count == esl::system::Interface::FileDescriptor::npos) {
			/* error occurred. drop responseHandler */
			responseHandler = nullptr;
		} else if(process(tmpBuffer, count) == false) {
			/* no more data to read desired. drop responseHandler */
			count = esl::system::Interface::FileDescriptor::npos;
		}
	}

	return count;
}

unsigned short ResponseHandler::getStatusCode() const {
	if(statusCode == 0) {
		auto iter = headers.find("Status");
		if(iter != headers.end()) {
			std::vector<std::string> values = esl::utility::String::split(iter->second, ' ');
			if(values.empty() == false) {
				statusCode = static_cast<unsigned short>(std::stoul(values.front()));
			}
		}
	}

	return statusCode;
}

std::map<std::string, std::string>& ResponseHandler::getHeaders() & {
	return headers;
}

std::map<std::string, std::string>&& ResponseHandler::getHeaders() && {
	return std::move(headers);
}

bool ResponseHandler::process(const char* contentData, std::size_t contentSize) {
	if(contentSize == esl::system::Interface::FileDescriptor::npos) {
		responseHandler = nullptr;
		return false;
	}

	if(readHeader) {
		buffer.append(contentData, contentSize);

		auto iter = buffer.begin();
		while(iter != buffer.end()) {
			if(*iter == '\n') {
				std::string line = std::string(buffer.begin(), iter);

				++iter;
				if(*iter == '\r') {
					++iter;
				}
				iter = buffer.erase(buffer.begin(), iter);

				if(line.empty()) {
					readHeader = false;
					break;
				}

				addHeader(std::move(line));
			}

			if(*iter == '\r') {
				std::string line = std::string(buffer.begin(), iter);

				++iter;
				if(*iter == '\n') {
					++iter;
				}
				iter = buffer.erase(buffer.begin(), iter);

				if(line.empty()) {
					readHeader = false;
					break;
				}

				addHeader(std::move(line));
			}
		}

		if(readHeader) {
			return true;
		}

		if(buffer.empty() == false) {
			contentData = buffer.data();
			contentSize = buffer.size();
		}
	}

	if(responseHandler == nullptr) {
		return false;
	}

	if(responseHandler->consumer(contentData, contentSize) == false) {
		responseHandler = nullptr;
		return false;
	}

	return true;
}

void ResponseHandler::addHeader(std::string line) {
	line = esl::utility::String::trim(std::move(line));

	auto iter = line.begin();
	for(; iter != line.end() && *iter != ' '; ++iter) {
		if(*iter == ':') {
			break;
		}
	}

	if(iter != line.end() && *iter == ':') {
		std::string key = std::string(line.begin(), iter);
		++iter;
		headers[key] = esl::utility::String::trim(std::string(iter, line.end()));
	}
	else {
		logger.warn << "Malformed header line in CGI response: \"" << line << "\"\n";
		statusCode = 500;
	}
}

} /* namespace consumer */
} /* namespace client */
} /* namespace cgi */
} /* namespace jerry */
#endif
