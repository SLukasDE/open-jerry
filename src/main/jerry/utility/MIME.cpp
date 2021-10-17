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

#include <jerry/utility/MIME.h>

#include <esl/utility/String.h>

#include <map>
#include <fstream>

namespace jerry {
namespace utility {
namespace {

std::map<std::string, std::string>& getExtensionToMimeType() {
	static std::map<std::string, std::string> extensionToMimeType = {
		{"bmp", "image/bmp"},
		{"bz", "application/x-bzip"},
		{"bz2", "application/x-bzip2"},
		{"css", "text/css"},
		{"gif", "image/gif"},
		{"gz", "application/x-compressed"},
		{"gzip", "application/x-gzip"},
		{"htm", "text/html"},
		{"html", "text/html"},
		{"jpg", "image/jpeg"},
		{"jpeg", "image/jpeg"},
		{"js", "text/javascript"},
		{"log", "text/plain"},
		{"mp3", "audio/mpeg3"},
		{"png", "image/png"},
		{"text", "text/plain"},
		{"txt", "text/plain"},
		{"tgz", "application/x-compressed"},
		{"tif", "image/tiff"},
		{"wav", "audio/wav"},
		{"woff", "font/woff"},
		{"woff2", "font/woff2"},
		{"xls", "application/excel"},
		{"xml", "text/xml"},
		{"zip", "application/zip"}
	};

	return extensionToMimeType;
}

} /* anonymous namespace */

void MIME::loadDefinition(const std::string& filename) {
	std::map<std::string, std::vector<std::string>> mimeTypeToExtensions = {};
	std::ifstream infile(filename);

	std::string line;
	while (std::getline(infile, line)) {
		auto columns = esl::utility::String::split(line, {{' '}, {'\t'}}, true);
		for(auto iter = columns.begin(); iter != columns.end(); ++iter) {
			if(iter->empty() || iter->at(0) == '#') {
				columns.erase(iter, columns.end());
				break;
			}
		}

		if(columns.empty()) {
			continue;
		}

		std::map<std::string, std::string>& extensionToMimeType = getExtensionToMimeType();

		if(columns[0] == "-") {
			for(std::size_t index = 1; index < columns.size(); ++index) {
				extensionToMimeType.erase(columns[index]);
			}
		}
		else {
			for(std::size_t index = 1; index < columns.size(); ++index) {
				extensionToMimeType[columns[index]] = columns[0];
			}
		}
	}
}

esl::utility::MIME MIME::byFilename(const std::string& filename) {
    std::size_t pos = filename.find_last_of(".");
    std::string fileExtension;

    /* Dateiendung gefunden */
    if(pos != std::string::npos) {
    	fileExtension = filename.substr(pos+1);
    }

    return byFileExtension(std::move(fileExtension));
}

esl::utility::MIME MIME::byFileExtension(std::string fileExtension) {
    const std::map<std::string, std::string>& extensionToMimeType = getExtensionToMimeType();

    auto iter = extensionToMimeType.find(fileExtension);
    if(iter != std::end(extensionToMimeType)) {
        return esl::utility::MIME(iter->second);
    }

    //std::transform(fileExtension.begin(), fileExtension.end(), fileExtension.begin(), tolower);
    fileExtension = esl::utility::String::toLower(std::move(fileExtension));
    iter = extensionToMimeType.find(fileExtension);
    if(iter != std::end(extensionToMimeType)) {
        return esl::utility::MIME(iter->second);
    }

    return esl::utility::MIME();
}

} /* namespace utility */
} /* namespace jerry */
