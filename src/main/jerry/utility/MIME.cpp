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

#include <jerry/utility/MIME.h>
#include <algorithm>

namespace jerry {
namespace utility {

MIME::MIME(std::string aMediaType, std::string aSubType) noexcept
: mediaType(aMediaType),
  subType(aSubType)
{ }

std::string MIME::getContentType() noexcept {
	return mediaType + "/" + subType;
}

MIME MIME::byFilename(const std::string& filename) {
    std::size_t pos = filename.find_last_of(".");
    std::string fileType;

    /* Dateiendung gefunden */
    if(pos != std::string::npos) {
        fileType = filename.substr(pos+1);

        std::transform(fileType.begin(), fileType.end(), fileType.begin(), tolower);
    }


    if(fileType == "bmp") {
        return MIME("image", "bmp");
    }
    else if(fileType == "bz") {
        return MIME("application", "x-bzip");
    }
    else if(fileType == "bz2") {
        return MIME("application", "x-bzip2");
    }
    else if(fileType == "css") {
        return MIME("text", "css");
    }
    else if(fileType == "gif") {
        return MIME("image", "gif");
    }
    else if(fileType == "gz") {
        return MIME("application", "x-compressed");
    }
    else if(fileType == "gzip") {
        return MIME("application", "x-gzip");
    }
    else if(fileType == "htm" || fileType == "html") {
        return MIME("text", "html");
    }
    else if(fileType == "jpg" || fileType == "jpeg") {
        return MIME("image", "jpeg");
    }
    else if(fileType == "js") {
        return MIME("text", "javascript");
    }
    else if(fileType == "log") {
        return MIME("text", "plain");
    }
    else if(fileType == "mp3") {
        return MIME("audio", "mpeg3");
    }
    else if(fileType == "png") {
        return MIME("image", "png");
    }
    else if(fileType == "text") {
        return MIME("text", "plain");
    }
    else if(fileType == "txt") {
        return MIME("text", "plain");
    }
    else if(fileType == "tgz") {
        return MIME("application", "x-compressed");
    }
    else if(fileType == "tif") {
        return MIME("image", "tiff");
    }
    else if(fileType == "wav") {
        return MIME("audio", "wav");
    }
    else if(fileType == "woff") {
        return MIME("font", "woff");
    }
    else if(fileType == "woff2") {
        return MIME("font", "woff2");
    }
    else if(fileType == "xls") {
        return MIME("application", "excel");
    }
    else if(fileType == "xml") {
        return MIME("text", "xml");
    }
    else if(fileType == "zip") {
        return MIME("application", "zip");
    }

    return MIME("text", "plain");
}

} /* namespace utility */
} /* namespace jerry */
