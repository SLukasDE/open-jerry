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

#include <jerry/builtin/basic/dump/RequestHandler.h>
#include <jerry/Logger.h>

#include <esl/io/Consumer.h>
#include <esl/io/Reader.h>
#include <esl/Stacktrace.h>

#include <stdexcept>

namespace jerry {
namespace builtin {
namespace basic {
namespace dump {

namespace {
Logger logger("jerry::builtin::basic::dump::RequestHandler");

class RequestConsumer : public esl::io::Consumer {
public:
	static esl::io::Input createInput(esl::com::basic::server::RequestContext& requestContext, esl::object::Interface::ObjectContext& objectContext);

	RequestConsumer(bool aShowContent)
	: showContent(aShowContent)
	{ }

	/* return: true for every kind of success and get called again for more content data
	 *         false for failure or to get not called again
	 */
	bool consume(esl::io::Reader& reader) override {
		if(showContent == false) {
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

private:
	bool showContent;
};

} /* anonymous namespace */

std::unique_ptr<esl::com::basic::server::requesthandler::Interface::RequestHandler> RequestHandler::createRequestHandler(const esl::module::Interface::Settings& settings) {
	return std::unique_ptr<esl::com::basic::server::requesthandler::Interface::RequestHandler>(new RequestHandler(settings));
}

RequestHandler::RequestHandler(const esl::module::Interface::Settings& settings) {
	for(const auto& setting : settings) {
		if(setting.first == "show-context") {
			if(setting.second == "true") {
				showContext = true;
			}
			else if(setting.second == "false") {
				showContext = false;
			}
			else {
				throw std::runtime_error("Unknown value \"" + setting.second + "\" for parameter key=\"" + setting.first + "\". Possible values are \"true\" or \"false\".");
			}
		}
		else if(setting.first == "show-content") {
			if(setting.second == "true") {
				showContent = true;
			}
			else if(setting.second == "false") {
				showContent = false;
			}
			else {
				throw std::runtime_error("Unknown value \"" + setting.second + "\" for parameter key=\"" + setting.first + "\". Possible values are \"true\" or \"false\".");
			}
		}
		else if(setting.first == "notifier") {
			notifiers.insert(setting.second);
		}
		else {
			throw esl::addStacktrace(std::runtime_error("Unknown parameter key=\"" + setting.first + "\" with value=\"" + setting.second + "\""));
		}
	}
}

esl::io::Input RequestHandler::accept(esl::com::basic::server::RequestContext& requestContext, esl::object::Interface::ObjectContext& objectContext) const {
	if(showContext) {
		logger.info << "Context:\n";
		for(const auto& entry : requestContext.getRequest().getValues()) {
			logger.info << "- \"" << entry.first << "\" = \"" << entry.second << "\"\n";
		}
	}

	if(showContent) {
		logger.info << "Content: \"";
	}

	return esl::io::Input(std::unique_ptr<esl::io::Consumer>(new RequestConsumer(showContent)));
}

std::set<std::string> RequestHandler::getNotifiers() const {
	return notifiers;
}

} /* namespace dump */
} /* namespace basic */
} /* namespace builtin */
} /* namespace jerry */
