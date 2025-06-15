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

#include <openjerry/builtin/http/filebrowser/RequestHandler.h>
#include <openjerry/Logger.h>
#include <openjerry/utility/MIME.h>

#include <esl/com/http/server/Connection.h>
#include <esl/com/http/server/exception/StatusCode.h>
#include <esl/com/http/server/Response.h>
#include <esl/io/input/Closed.h>
#include <esl/io/output/Memory.h>
#include <esl/io/output/String.h>
#include <esl/system/Stacktrace.h>
#include <esl/utility/HttpMethod.h>
#include <esl/utility/MIME.h>
#include <esl/utility/String.h>

#include <cstring>
#include <filesystem>
#include <fstream>
#include <sstream>
#include <stdexcept>

namespace openjerry {
namespace builtin {
namespace http {
namespace filebrowser {

namespace {
Logger logger("openjerry::builtin::http::filebrowser::RequestHandler");

const std::string PAGE_301(
		"<!DOCTYPE html>\n"
		"<html>\n"
		"<head>\n"
		"<title>301</title>\n"
		"</head>\n"
		"<body>\n"
		"<h1>301</h1>\n"
		"</body>\n"
		"</html>\n");
} /* anonymous namespace */

std::unique_ptr<esl::com::http::server::RequestHandler> RequestHandler::createRequestHandler(const std::vector<std::pair<std::string, std::string>>& settings) {
	return std::unique_ptr<esl::com::http::server::RequestHandler>(new RequestHandler(settings));
}

RequestHandler::RequestHandler(const std::vector<std::pair<std::string, std::string>>& settings) {
	bool hasBrowsable = false;

	for(const auto& setting : settings) {
		if(setting.first == "browsable") {
			if(hasBrowsable) {
				throw std::runtime_error("Multiple definition of attribute 'browsable'");
			}
			if(setting.second == "true") {
				browsable = true;
			}
			else if(setting.second == "false") {
				browsable = false;
			}
			else {
				throw std::runtime_error("Unknown value \"" + setting.second + "\" for parameter key=\"" + setting.first + "\". Possible values are \"true\" or \"false\".");
			}
			hasBrowsable = true;
		}
		else if(setting.first == "path") {
#if 0
			path = setting.second;
			while(!path.empty() && path.at(path.size()-1) == '/') {
				path = path.substr(0, path.size()-1);
			}
#endif
			path = esl::utility::String::rtrim(setting.second, '/');
		}
		else if(setting.first == "default") {
			defaults.insert(setting.second);
		}
		else if(setting.first == "ignore-error") {
			if(setting.second == "true") {
				ignoreError = true;
			}
			else if(setting.second == "false") {
				ignoreError = false;
			}
			else {
				throw std::runtime_error("Unknown value \"" + setting.second + "\" for parameter key=\"" + setting.first + "\". Possible values are \"true\" or \"false\".");
			}
		}
		/*
		else if(setting.first == "accept-all") {
			setAcceptAll(key, value, &Settings::setShowException);
		}
		*/
		else {
			throw std::runtime_error("Unknown parameter key=\"" + setting.first + "\" with value=\"" + setting.second + "\"");
		}
	}
}

esl::io::Input RequestHandler::accept(esl::com::http::server::RequestContext& requestContext) const {
	logger.trace << "Incoming request with method \"" << requestContext.getRequest().getMethod().toString() << "\".\n";
	if(requestContext.getRequest().getMethod() != esl::utility::HttpMethod::Type::httpGet) {
		if(ignoreError) {
			logger.trace << "Ignoring request\n";
			return esl::io::Input();
		}

		logger.trace << "Method \"" << requestContext.getRequest().getMethod().toString() << "\" is not supported\n";
		throw esl::com::http::server::exception::StatusCode(405);
	}

	// skip leading slashes to make operator "/=" work correctly
	const char* urlPath = requestContext.getPath().c_str();
	while(urlPath && *urlPath == '/') {
		++urlPath;
	}
	std::filesystem::path fullPath = path / urlPath;

	logger.trace << "Request   '" << requestContext.getPath() << "'\n";
	logger.trace << "Base path '" << path << "'\n";
	logger.trace << "Full path '" << fullPath << "'\n";

	if(!std::filesystem::exists(fullPath)) {
		logger.trace << "Original path " << fullPath << " not exists.\n";
		if(ignoreError) {
			return esl::io::Input();
		}
		else {
			throw esl::com::http::server::exception::StatusCode(404);
		}
	}

	if(std::filesystem::is_directory(fullPath)) {
		logger.trace << "Full path is a directory\n";
		// if requestContext.getPath() pointing to a directury but not ending with character '/', then we will send a redirect to an URL ending with '/'
    	if(requestContext.getPath().empty() || requestContext.getPath().at(requestContext.getPath().size()-1) != '/') {
			logger.trace << "Sending redirect...\n";
    		esl::com::http::server::Response response(301, esl::utility::MIME::Type::textHtml);
    		response.addHeader("Location", requestContext.getRequest().getPath() + "/");
    		esl::io::Output output = esl::io::output::Memory::create(PAGE_301.data(), PAGE_301.size());
    		requestContext.getConnection().send(response, std::move(output));

    		return esl::io::input::Closed::create();
    	}

		logger.trace << "enrich with default...\n";
		for(const auto& defaultFile : defaults) {
			logger.trace << "enrich full path with default file '" << defaultFile << "'\n";
			std::filesystem::path file = fullPath / defaultFile;
			if(std::filesystem::exists(file) && std::filesystem::is_regular_file(file)) {
				logger.trace << "File exists! Update full path...\n";
				fullPath = file;
				break;
			}
			logger.trace << "Skip file, because it does not exists!\n";
		}
		logger.trace << "New full path '" << fullPath << "'\n";
	}


	if(std::filesystem::is_directory(fullPath)) {
		logger.trace << "Path " << fullPath << " is a directory\n";
    	if(browsable) {
    		std::string outputContent;

    		outputContent += "<html><head><title>Directory of " + requestContext.getPath() + "</title></head><body><h2>Directory of " + requestContext.getPath() + "</h2><br/>";
    		outputContent += "<a href=\"..\">..</a><br/>\n";

    		for (std::filesystem::directory_iterator itr(fullPath); itr!=std::filesystem::directory_iterator(); ++itr) {
    			if(std::filesystem::is_directory(itr->path())) {
    				outputContent += "<a href=\"" + itr->path().filename().generic_string() + "/\">" + itr->path().filename().generic_string() + "/</a><br/>\n";
    			}
    			else if(std::filesystem::is_regular_file(itr->path())) {
    				outputContent += "<a href=\"" + itr->path().filename().generic_string() + "\">" + itr->path().filename().generic_string() + "</a><br/>\n";
    			}
    			else {
    				outputContent += itr->path().filename().generic_string() + "<br/>\n";
    			}
    		}
    		outputContent += "</body></html>";

    		esl::com::http::server::Response response(200, esl::utility::MIME::Type::textHtml);
    		esl::io::Output output = esl::io::output::String::create(std::move(outputContent));
    		requestContext.getConnection().send(response, std::move(output));
    		return esl::io::input::Closed::create();
    	}

		logger.trace << "Directory " << fullPath << " not browsable.\n";
    	if(ignoreError) {
    		return esl::io::Input();
    	}

		throw esl::com::http::server::exception::StatusCode(403);
	}

	logger.trace << "Path " << fullPath << " is a file\n";
	if(std::filesystem::is_regular_file(fullPath)) {
		esl::utility::MIME mime = openjerry::utility::MIME::byFilename(fullPath.generic_string());
		esl::com::http::server::Response response(200, mime);
		requestContext.getConnection().sendFile(response, fullPath.generic_string());
		return esl::io::input::Closed::create();
	}

	logger.warn << "Path " << fullPath << " is not a regular file\n";
	if(ignoreError) {
		return esl::io::Input();
	}

	throw esl::com::http::server::exception::StatusCode(422);
}

} /* namespace filebrowser */
} /* namespace http */
} /* namespace builtin */
} /* namespace openjerry */
