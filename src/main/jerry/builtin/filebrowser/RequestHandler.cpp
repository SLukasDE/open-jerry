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

#include <jerry/builtin/filebrowser/RequestHandler.h>
#include <jerry/utility/MIME.h>
#include <jerry/Logger.h>

#include <eslx/utility/Directory.h>

#include <esl/http/server/Connection.h>
#include <esl/http/server/ResponseDynamic.h>
#include <esl/http/server/ResponseFile.h>
#include <esl/http/server/ResponseStatic.h>
#include <esl/http/server/exception/StatusCode.h>
#include <esl/logging/Logger.h>
#include <esl/Stacktrace.h>
#include <esl/logging/Level.h>

#include <memory>
#include <cstring>
#include <fstream>
#include <sstream>
#include <set>

namespace jerry {
namespace builtin {
namespace filebrowser {

namespace {
Logger logger("jerry::builtin::filebrowser::RequestHandler");


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
}

std::unique_ptr<esl::http::server::requesthandler::Interface::RequestHandler> RequestHandler::create(esl::http::server::RequestContext& requestContext) {
	if(requestContext.getRequest().getMethod() != "GET") {
		return nullptr;
	}
	const Settings* settings = dynamic_cast<Settings*>(requestContext.getObject(""));
	if(settings == nullptr) {
		return nullptr;
	}

	std::string path = settings->getPath() + requestContext.getPath();
	eslx::utility::Directory directory(path);
	bool isDirectory;

	try {
		isDirectory = directory.getEntry().isDirectory;
	}
	catch(...) {
		throw esl::http::server::exception::StatusCode(404);
	}

	return std::unique_ptr<esl::http::server::requesthandler::Interface::RequestHandler>(new RequestHandler(requestContext, *settings, isDirectory));
}

RequestHandler::RequestHandler(esl::http::server::RequestContext& aRequestContext, const Settings& aSettings, bool isDirectory)
: esl::http::server::requesthandler::Interface::RequestHandler(),
  requestContext(aRequestContext),
  settings(aSettings)
{
	std::string path = settings.getPath() + requestContext.getPath();
	std::vector<eslx::utility::Directory::Entry> entries;

    if(isDirectory) {
    	eslx::utility::Directory directory(path);

		// Wenn getUrl auf ein Directory zeigt, aber nicht auf "/" endet, dann sende ein Redirect auf URL mit Endung "/"
    	if(requestContext.getPath().size() == 0 || requestContext.getPath().at(requestContext.getPath().size()-1) != '/') {
    		outputContent.clear();
    		std::unique_ptr<esl::http::server::ResponseStatic> response;
    		response.reset(new esl::http::server::ResponseStatic(301, esl::utility::MIME::textHtml, PAGE_301.data(), PAGE_301.size()));
    		response->addHeader("Location", requestContext.getRequest().getPath() + "/");
    		requestContext.getConnection().sendResponse(std::move(response));
    		//return false;
    		return;
    	}

		entries = directory.scan(false);

		const std::set<std::string>& defaults = settings.getDefaults();
		for(const auto& entry : entries) {
			if(defaults.find(entry.name) != defaults.end()) {
				path += "/" + entry.name;
				isDirectory = false;
				break;
			}
		}
    }

    if(isDirectory) {
    	if(settings.isBrowsable()) {
    		outputContent.clear();
    		outputContent += "<html><head><title>Directory of " + requestContext.getPath() + "</title></head><body><h2>Directory of " + requestContext.getPath() + "</h2><br/>";
    		outputContent += "<a href=\"..\">..</a><br/>\n";

    		for(const auto& entry : entries) {
    			if(entry.isDirectory) {
    				outputContent += "<a href=\"" + entry.name + "/\">" + entry.name + "/</a><br/>\n";
    			}
    			else {
    				outputContent += "<a href=\"" + entry.name + "\">" + entry.name + "</a><br/>\n";
    			}
    		}
    		outputContent += "</body></html>";

    		std::unique_ptr<esl::http::server::ResponseDynamic> response;
    		auto lambda = [this](char* buffer, std::size_t count) { return getData(buffer, count); };
    		response.reset(new esl::http::server::ResponseDynamic(200, esl::utility::MIME::textHtml, lambda ));
    		requestContext.getConnection().sendResponse(std::move(response));
    	}
    	else {
    		throw esl::http::server::exception::StatusCode(403);
    	}
	}
    else {
    	esl::utility::MIME mime = utility::MIME::byFilename(path);
		std::unique_ptr<esl::http::server::ResponseFile> response(new esl::http::server::ResponseFile(settings.getHttpStatus(), mime, std::move(path)));
		requestContext.getConnection().sendResponse(std::move(response));

#if 0
        std::ifstream fileStream(path, std::ios::binary);
        if(! fileStream.good()) {
        	throw esl::addStacktrace(std::runtime_error("cannot open file"));
        }

        std::vector<char> fileBuffer;
        fileBuffer = std::vector<char>(std::istreambuf_iterator<char>(fileStream), {});
		outputContent.clear();
		outputContent.append(&fileBuffer[0], fileBuffer.size());

		std::unique_ptr<esl::http::server::ResponseDynamic> response;
		auto lambda = [this](char* buffer, std::size_t count) { return getData(buffer, count); };
		response.reset(new esl::http::server::ResponseDynamic(settings.getHttpStatus(), utility::MIME::byFilename(path).getContentType(), lambda ));
		requestContext.getConnection().sendResponse(std::move(response));
#endif
    }
}

int RequestHandler::getData(char* buffer, std::size_t bufferSize) {
    std::size_t remainingSize = outputContent.size() - outputPos;

    if(bufferSize > remainingSize) {
        bufferSize = remainingSize;
    }
    std::memcpy(buffer, &outputContent.data()[outputPos], bufferSize);
    outputPos += bufferSize;

    /* Hier wissen wir, dass keine Daten mehr zu senden sind, wenn bufferSize == 0 ist */
    /* Daher geben wir dann auch -1 zurueck - was dem Aufrufer signalisiert, dass keine Daten mehr zu senden sind */
    if(bufferSize == 0) {
        return -1;
    }
    return bufferSize;
}

} /* namespace filebrowser */
} /* namespace builtin */
} /* namespace jerry */
