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

#include <jerry/builtin/messaging/dump/RequestHandler.h>
#include <jerry/Logger.h>

namespace jerry {
namespace builtin {
namespace messaging {
namespace dump {

namespace {
Logger logger("jerry::builtin::messaging::dump::RequestHandler");
}

esl::io::Input RequestHandler::createInput(esl::messaging::server::RequestContext& requestContext) {
	const Settings* settings = requestContext.findObject<Settings>("");
	if(settings == nullptr) {
		logger.warn << "Settings object is missing\n";
		return esl::io::Input();
	}

	if(settings->getShowContext()) {
		logger.info << "Context:\n";
		for(const auto& entry : requestContext.getRequest().getValues()) {
			logger.info << "- \"" << entry.first << "\" = \"" << entry.second << "\"\n";
		}
	}

	if(settings->getShowContent()) {
		logger.info << "Content: \"";
	}

	return esl::io::Input(std::unique_ptr<esl::io::Consumer>(new RequestHandler(*settings)));
}

const std::set<std::string>& RequestHandler::getNotifiers(const esl::object::ObjectContext& objectContext) {
	const Settings* settings = objectContext.findObject<Settings>("");
	return settings->getNotifiers();
}

RequestHandler::RequestHandler(const Settings& aSettings)
: settings(aSettings)
{ }

/* return: true for every kind of success and get called again for more content data
 *         false for failure or to get not called again
 */
bool RequestHandler::consume(esl::io::Reader& reader)  {
	if(settings.getShowContent() == false) {
		return false;
	}

	char data[1024];
	std::size_t len = reader.read(data, 1024);

	if(len == esl::io::Reader::npos) {
		logger.info << "\"\n";
		return false;
	}

	logger.info << std::string(data, len);
	return true;
}

} /* namespace dump */
} /* namespace messaging */
} /* namespace builtin */
} /* namespace jerry */
