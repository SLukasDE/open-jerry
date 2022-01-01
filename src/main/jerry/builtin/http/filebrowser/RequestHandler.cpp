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

#include <jerry/builtin/http/filebrowser/RequestHandler.h>
#include <jerry/utility/MIME.h>
#include <jerry/Logger.h>

#include <esl/io/input/Closed.h>
#include <esl/io/output/Memory.h>
#include <esl/io/output/String.h>
#include <esl/com/http/server/Connection.h>
#include <esl/com/http/server/exception/StatusCode.h>
#include <esl/Stacktrace.h>
#include <esl/utility/String.h>

#include <boost/filesystem/operations.hpp>
#include <boost/filesystem/path.hpp>

#include <stdexcept>
#include <cstring>
#include <fstream>
#include <sstream>
#include <memory>

namespace jerry {
namespace builtin {
namespace http {
namespace filebrowser {

namespace {
Logger logger("jerry::builtin::http::filebrowser::RequestHandler");

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

std::unique_ptr<esl::com::http::server::requesthandler::Interface::RequestHandler> RequestHandler::createRequestHandler(const esl::object::Interface::Settings& settings) {
	return std::unique_ptr<esl::com::http::server::requesthandler::Interface::RequestHandler>(new RequestHandler(settings));
}

RequestHandler::RequestHandler(const esl::object::Interface::Settings& settings) {
	for(const auto& setting : settings) {
		if(setting.first == "browsable") {
			if(setting.second == "true") {
				browsable = true;
			}
			else if(setting.second == "false") {
				browsable = false;
			}
			else {
				throw std::runtime_error("Unknown value \"" + setting.second + "\" for parameter key=\"" + setting.first + "\". Possible values are \"true\" or \"false\".");
			}
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
		else if(setting.first == "ignoreError") {
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
			throw esl::addStacktrace(std::runtime_error("Unknown parameter key=\"" + setting.first + "\" with value=\"" + setting.second + "\""));
		}
	}
}

esl::io::Input RequestHandler::accept(esl::com::http::server::RequestContext& requestContext, esl::object::Interface::ObjectContext& objectContext) const {
	if(requestContext.getRequest().getMethod() != "GET") {
		if(ignoreError) {
			return esl::io::Input();
		}

		logger.trace << "Method \"" << requestContext.getRequest().getMethod() << "\" is not supported\n";
		throw esl::com::http::server::exception::StatusCode(405);
	}

#if 0
	boost::filesystem::path path = path + "/" + requestContext.getPath();
#endif
	logger.trace << "Setting path is \"" << path << "\"\n";
	boost::filesystem::path fullPath = path + requestContext.getPath();

	if(!boost::filesystem::exists(fullPath)) {
		logger.trace << "Original path " << fullPath << " not exists.\n";
		if(ignoreError) {
			return esl::io::Input();
		}
		else {
			throw esl::com::http::server::exception::StatusCode(404);
		}
	}

	if(boost::filesystem::is_directory(fullPath)) {
		logger.trace << "Original path " << path << " is a directory\n";
		// Wenn getUrl auf ein Directory zeigt, aber nicht auf "/" endet, dann sende ein Redirect auf URL mit Endung "/"
    	if(requestContext.getPath().size() == 0 || requestContext.getPath().at(requestContext.getPath().size()-1) != '/') {
    		esl::com::http::server::Response response(301, esl::utility::MIME::textHtml);
    		response.addHeader("Location", requestContext.getRequest().getPath() + "/");
    		esl::io::Output output = esl::io::output::Memory::create(PAGE_301.data(), PAGE_301.size());
    		requestContext.getConnection().send(response, std::move(output));

    		return esl::io::input::Closed::create();
    	}

		for(const auto& defaultFile : defaults) {
			boost::filesystem::path file = fullPath / defaultFile;
			if(boost::filesystem::exists(file) && boost::filesystem::is_regular_file(file)) {
				fullPath = file;
				break;
			}
		}
	}


	if(boost::filesystem::is_directory(fullPath)) {
		logger.trace << "Path " << fullPath << " is a directory\n";
    	if(browsable) {
    		std::string outputContent;

    		outputContent += "<html><head><title>Directory of " + requestContext.getPath() + "</title></head><body><h2>Directory of " + requestContext.getPath() + "</h2><br/>";
    		outputContent += "<a href=\"..\">..</a><br/>\n";

    		for (boost::filesystem::directory_iterator itr(fullPath); itr!=boost::filesystem::directory_iterator(); ++itr) {
    			if(boost::filesystem::is_directory(itr->path())) {
    				outputContent += "<a href=\"" + itr->path().filename().generic_string() + "/\">" + itr->path().filename().generic_string() + "/</a><br/>\n";
    			}
    			else if(boost::filesystem::is_regular_file(itr->path())) {
    				outputContent += "<a href=\"" + itr->path().filename().generic_string() + "\">" + itr->path().filename().generic_string() + "</a><br/>\n";
    			}
    			else {
    				outputContent += itr->path().filename().generic_string() + "<br/>\n";
    			}
    		}
    		outputContent += "</body></html>";

    		esl::com::http::server::Response response(200, esl::utility::MIME::textHtml);
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
	if(boost::filesystem::is_regular_file(fullPath)) {
    	esl::utility::MIME mime = utility::MIME::byFilename(fullPath.generic_string());
		esl::com::http::server::Response response(200, mime);
		requestContext.getConnection().send(response, fullPath);
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
} /* namespace jerry */
