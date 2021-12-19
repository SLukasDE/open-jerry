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
#include <esl/io/Consumer.h>
#include <esl/io/Reader.h>
#include <esl/Stacktrace.h>

#include <stdexcept>

#include <chrono>
#include <thread>

namespace jerry {
namespace builtin {
namespace basic {
namespace echo {

namespace {
Logger logger("jerry::builtin::basic::echo::RequestHandler");

class RequestConsumer : public esl::io::Consumer {
public:
	RequestConsumer(esl::com::basic::server::RequestContext& aRequestContext, esl::com::basic::client::Interface::ConnectionFactory& aConnectionFactory, unsigned long aMsDelay)
	: requestContext(aRequestContext),
	  msDelay(aMsDelay),
	  connectionFactory(aConnectionFactory)
	{ }

	/* return: true for every kind of success and get called again for more content data
	 *         false for failure or to get not called again
	 */
	bool consume(esl::io::Reader& reader) override {
		char data[1024];
		std::size_t len = reader.read(data, 1024);

		if(len == esl::io::Reader::npos) {
			logger.info << "Echo - sleep " << msDelay << "ms\n";
			std::this_thread::sleep_for(std::chrono::milliseconds(msDelay));

			std::unique_ptr<esl::com::basic::client::Interface::Connection> connection = connectionFactory.createConnection();
			if(connection) {
				std::unique_ptr<esl::io::Producer> producer(new esl::io::output::String(std::move(message)));

				logger.info << "Echo - send echo\n";
				connection->send({}, esl::io::Output(std::move(producer)), esl::io::Input());
			}
			else {
				logger.error << "Echo - send echo failed because connection could not be established\n";
			}
			return false;
		}

		message += std::string(data, len);
		return true;
	}

private:
	esl::com::basic::server::RequestContext& requestContext;
	unsigned long msDelay;
	esl::com::basic::client::Interface::ConnectionFactory& connectionFactory;
	std::string message;
};

} /* anonymous namespace */

std::unique_ptr<esl::com::basic::server::requesthandler::Interface::RequestHandler> RequestHandler::createRequestHandler(const esl::module::Interface::Settings& settings) {
	return std::unique_ptr<esl::com::basic::server::requesthandler::Interface::RequestHandler>(new RequestHandler(settings));
}

RequestHandler::RequestHandler(const esl::module::Interface::Settings& settings) {
	for(const auto& setting : settings) {
		if(setting.first == "notifier") {
			notifiers.insert(setting.second);
		}
		else if(setting.first == "delay-ms") {
			try {
				msDelay = std::stoul(setting.second);
			}
			catch(...) {
				throw esl::addStacktrace(std::runtime_error("Invalid value \"" + setting.second + "\" for parameter key=\"" + setting.first + "\". Value must be an integer"));
			}
		}
		else if(setting.first == "connection-factory-id") {
			connectionFactoryId = setting.second;
		}
		else {
			throw esl::addStacktrace(std::runtime_error("Unknown parameter key=\"" + setting.first + "\" with value=\"" + setting.second + "\""));
		}
	}
}

void RequestHandler::initializeContext(esl::object::Interface::ObjectContext& objectContext) {
	connectionFactory = objectContext.findObject<esl::com::basic::client::Interface::ConnectionFactory>(connectionFactoryId);
	if(connectionFactory == nullptr) {
		throw esl::addStacktrace(std::runtime_error("Cannot find basic-client-factory with id \"" + connectionFactoryId + "\""));
	}
}

esl::io::Input RequestHandler::accept(esl::com::basic::server::RequestContext& requestContext, esl::object::Interface::ObjectContext& objectContext) const {
	return esl::io::Input(std::unique_ptr<esl::io::Consumer>(new RequestConsumer(requestContext, *connectionFactory, msDelay)));
}

std::set<std::string> RequestHandler::getNotifiers() const {
	return notifiers;
}

} /* namespace echo */
} /* namespace basic */
} /* namespace builtin */
} /* namespace jerry */
