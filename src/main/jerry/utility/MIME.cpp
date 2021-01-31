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
#include <algorithm>

namespace jerry {
namespace utility {

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
    std::transform(fileExtension.begin(), fileExtension.end(), fileExtension.begin(), tolower);

    if(fileExtension == "bmp") {
        return esl::utility::MIME("image/bmp");
    }
    else if(fileExtension == "bz") {
        return esl::utility::MIME("application/x-bzip");
    }
    else if(fileExtension == "bz2") {
        return esl::utility::MIME("application/x-bzip2");
    }
    else if(fileExtension == "css") {
        return esl::utility::MIME("text/css");
    }
    else if(fileExtension == "gif") {
        return esl::utility::MIME("image/gif");
    }
    else if(fileExtension == "gz") {
        return esl::utility::MIME("application/x-compressed");
    }
    else if(fileExtension == "gzip") {
        return esl::utility::MIME("application/x-gzip");
    }
    else if(fileExtension == "htm" || fileExtension == "html") {
        return esl::utility::MIME("text/html");
    }
    else if(fileExtension == "jpg" || fileExtension == "jpeg") {
        return esl::utility::MIME("image/jpeg");
    }
    else if(fileExtension == "js") {
        return esl::utility::MIME("text/javascript");
    }
    else if(fileExtension == "log") {
        return esl::utility::MIME(esl::utility::MIME::textPlain);
    }
    else if(fileExtension == "mp3") {
        return esl::utility::MIME("audio/mpeg3");
    }
    else if(fileExtension == "png") {
        return esl::utility::MIME("image/png");
    }
    else if(fileExtension == "text") {
        return esl::utility::MIME(esl::utility::MIME::textPlain);
    }
    else if(fileExtension == "txt") {
        return esl::utility::MIME(esl::utility::MIME::textPlain);
    }
    else if(fileExtension == "tgz") {
        return esl::utility::MIME("application/x-compressed");
    }
    else if(fileExtension == "tif") {
        return esl::utility::MIME("image/tiff");
    }
    else if(fileExtension == "wav") {
        return esl::utility::MIME("audio/wav");
    }
    else if(fileExtension == "woff") {
        return esl::utility::MIME("font/woff");
    }
    else if(fileExtension == "woff2") {
        return esl::utility::MIME("font/woff2");
    }
    else if(fileExtension == "xls") {
        return esl::utility::MIME("application/excel");
    }
    else if(fileExtension == "xml") {
        return esl::utility::MIME(esl::utility::MIME::textXml);
    }
    else if(fileExtension == "zip") {
        return esl::utility::MIME("application/zip");
    }

    return esl::utility::MIME(esl::utility::MIME::textPlain);
}

} /* namespace utility */
} /* namespace jerry */
