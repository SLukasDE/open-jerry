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

#include <jerry/builtin/basic/echo/RequestHandler.h>
#include <jerry/Logger.h>

#include <esl/com/basic/client/Interface.h>
#include <esl/io/output/String.h>
#include <esl/io/Producer.h>
#include <esl/io/Output.h>
#include <esl/Stacktrace.h>

#include <stdexcept>

#include <chrono>
#include <thread>
#include <memory>

namespace jerry {
namespace builtin {
namespace basic {
namespace echo {

namespace {
Logger logger("jerry::builtin::basic::echo::RequestHandler");
}

esl::io::Input RequestHandler::createInput(esl::com::basic::server::RequestContext& requestContext) {
	Settings* settings = requestContext.findObject<Settings>("");
	if(settings == nullptr) {
		logger.warn << "Settings object is missing\n";
		return esl::io::Input();
	}

	return esl::io::Input(std::unique_ptr<esl::io::Consumer>(new RequestHandler(requestContext, *settings)));
}

const std::set<std::string>& RequestHandler::getNotifiers(const esl::object::ObjectContext& objectContext) {
	const Settings* settings = objectContext.findObject<Settings>("");
	return settings->getNotifiers();
}

RequestHandler::RequestHandler(esl::com::basic::server::RequestContext& aRequestContext, Settings& aSettings)
: requestContext(aRequestContext),
  settings(aSettings)
{ }

/* return: true for every kind of success and get called again for more content data
 *         false for failure or to get not called again
 */
bool RequestHandler::consume(esl::io::Reader& reader) {
	char data[1024];
	std::size_t len = reader.read(data, 1024);

	if(len == esl::io::Reader::npos) {
		logger.info << "Echo - sleep " << settings.getMSDelay() << "ms\n";
		std::this_thread::sleep_for(std::chrono::milliseconds(settings.getMSDelay()));

		std::unique_ptr<esl::com::basic::client::Interface::Connection> connection = settings.createConnection();
		if(connection) {
			std::unique_ptr<esl::io::Producer> producer(new esl::io::output::String(std::move(message)));

			logger.info << "Echo - send echo\n";
			connection->send(esl::io::Output(std::move(producer)));
		}
		else {
			logger.info << "Echo - send echo failed because connection could not be established\n";
		}
		return false;
	}

	message += std::string(data, len);
	return true;
}


} /* namespace echo */
} /* namespace basic */
} /* namespace builtin */
} /* namespace jerry */
