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

#include <openjerry/builtin/http/self/RequestHandler.h>
#include <openjerry/Logger.h>

#include <esl/com/http/server/Request.h>
#include <esl/com/http/server/Response.h>
#include <esl/io/input/String.h>
#include <esl/io/output/String.h>
#include <esl/io/Writer.h>
#include <esl/system/Stacktrace.h>
#include <esl/utility/MIME.h>

#include <stdexcept>

namespace openjerry {
namespace builtin {
namespace http {
namespace self {

namespace {
Logger logger("openjerry::builtin::http::self::RequestHandler");

class InputHandler : public esl::io::input::String {
public:
	using ProcessHandler = void (InputHandler::*)();

	InputHandler(esl::com::http::server::RequestContext& aRequestContext)
	: requestContext(aRequestContext)
	{ }

	void process() override {
		std::string content;

		content += "LOCAL_PATH:     " + requestContext.getPath() + "\n";

		const auto& request = requestContext.getRequest();
		content += "FULL_PATH:      " + request.getPath() + "\n";
		content += "HTTP_VERSION:   " + request.getHTTPVersion() + "\n";
		content += "METHOD:         " + request.getMethod().toString() + "\n";
		content += "HOST_ADDRESS:   " + request.getHostAddress() + "\n";
		content += "HOST_PORT:      " + std::to_string(request.getHostPort()) + "\n";
		content += "REMOTE_ADDRESS: " + request.getRemoteAddress() + "\n";
		content += "REMOTE_PORT:    " + std::to_string(request.getRemotePort()) + "\n";
		content += "CONTENT_TYPE:   " + request.getContentType().toString() + "\n";
		content += "\n";
		content += "HTTP headers:\n";
		content += "\n";

		const std::map<std::string, std::string>& headers = requestContext.getRequest().getHeaders();
		for(const auto& header : headers) {
			content += header.first + "=" + header.second + "\n";
		}

		content += "\n";
		content += "HTTP body: (" + std::to_string(getString().size()) + " bytes)\n";
		content += "\n";
		content += getString();

		esl::com::http::server::Response response(200, esl::utility::MIME::Type::textPlain);
		esl::io::Output output = esl::io::output::String::create(std::move(content));
		requestContext.getConnection().send(response, std::move(output));
	}

private:
    esl::com::http::server::RequestContext& requestContext;
};

} /* anonymous namespace */

std::unique_ptr<esl::com::http::server::RequestHandler> RequestHandler::createRequestHandler(const std::vector<std::pair<std::string, std::string>>& settings) {
	for(const auto& setting : settings) {
		throw std::runtime_error("Unknown parameter key=\"" + setting.first + "\" with value=\"" + setting.second + "\"");
	}
	return std::unique_ptr<esl::com::http::server::RequestHandler>(new RequestHandler());
}

esl::io::Input RequestHandler::accept(esl::com::http::server::RequestContext& requestContext) const {
	return esl::io::Input(std::unique_ptr<esl::io::Writer>(new InputHandler(requestContext)));
}

} /* namespace self */
} /* namespace http */
} /* namespace builtin */
} /* namespace openjerry */
