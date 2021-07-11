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

#include <jerry/builtin/http/filebrowser/RequestHandler.h>
#include <jerry/builtin/http/filebrowser/Settings.h>
#include <jerry/utility/MIME.h>
#include <jerry/Logger.h>

#include <esl/io/input/Closed.h>
#include <esl/io/output/Memory.h>
#include <esl/io/output/String.h>
#include <esl/com/http/server/Connection.h>
#include <esl/com/http/server/exception/StatusCode.h>
#include <esl/Stacktrace.h>

#include <boost/filesystem/operations.hpp>
#include <boost/filesystem/path.hpp>

#include <string>
#include <cstring>
#include <fstream>
#include <sstream>
#include <set>
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

esl::io::Input RequestHandler::createRequestHandler(esl::com::http::server::RequestContext& requestContext) {
	const Settings* settings = requestContext.findObject<Settings>();

	if(settings == nullptr) {
		logger.warn << "Settings object missing\n";
		throw esl::com::http::server::exception::StatusCode(500);
	}

	if(requestContext.getRequest().getMethod() != "GET") {
		if(settings->getIgnoreError()) {
			return esl::io::Input();
		}

		logger.warn << "Method \"" << requestContext.getRequest().getMethod() << "\" is not supported\n";
		throw esl::com::http::server::exception::StatusCode(405);
	}

	boost::filesystem::path path = settings->getPath() + requestContext.getPath();
	if(!boost::filesystem::exists(path)) {
		if(settings->getIgnoreError()) {
			return esl::io::Input();
		}
		else {
			throw esl::com::http::server::exception::StatusCode(404);
		}
	}

	if(boost::filesystem::is_directory(path)) {
		// Wenn getUrl auf ein Directory zeigt, aber nicht auf "/" endet, dann sende ein Redirect auf URL mit Endung "/"
    	if(requestContext.getPath().size() == 0 || requestContext.getPath().at(requestContext.getPath().size()-1) != '/') {
    		esl::com::http::server::Response response(301, esl::utility::MIME::textHtml);
    		response.addHeader("Location", requestContext.getRequest().getPath() + "/");
    		esl::io::Output output = esl::io::output::Memory::create(PAGE_301.data(), PAGE_301.size());
    		requestContext.getConnection().send(response, std::move(output));

    		return esl::io::input::Closed::create();
    	}

		for(const auto& defaultFile : settings->getDefaults()) {
			boost::filesystem::path file = path / defaultFile;
			if(boost::filesystem::exists(file) && boost::filesystem::is_regular_file(file)) {
				path = file;
				break;
			}
		}
	}


	if(boost::filesystem::is_directory(path)) {
    	if(settings->isBrowsable()) {
    		std::string outputContent;

    		outputContent += "<html><head><title>Directory of " + requestContext.getPath() + "</title></head><body><h2>Directory of " + requestContext.getPath() + "</h2><br/>";
    		outputContent += "<a href=\"..\">..</a><br/>\n";

    		for (boost::filesystem::directory_iterator itr(path); itr!=boost::filesystem::directory_iterator(); ++itr) {
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

    	if(settings->getIgnoreError()) {
    		return esl::io::Input();
    	}

		throw esl::com::http::server::exception::StatusCode(403);
	}

	if(boost::filesystem::is_regular_file(path)) {
    	esl::utility::MIME mime = utility::MIME::byFilename(path.generic_string());
		esl::com::http::server::Response response(200, mime);
		requestContext.getConnection().send(response, path);
		return esl::io::input::Closed::create();
	}

	if(settings->getIgnoreError()) {
		return esl::io::Input();
	}

	throw esl::com::http::server::exception::StatusCode(422);
}

std::unique_ptr<esl::object::Interface::Object> RequestHandler::createSettings(const esl::object::Interface::Settings& settings) {
	return std::unique_ptr<esl::object::Interface::Object>(new Settings(settings));
}

} /* namespace filebrowser */
} /* namespace http */
} /* namespace builtin */
} /* namespace jerry */
