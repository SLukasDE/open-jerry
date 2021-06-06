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

#include <jerry/engine/basic/server/RequestContext.h>
#include <jerry/engine/basic/server/Context.h>
#include <jerry/engine/basic/server/Writer.h>
#include <jerry/engine/ExceptionHandler.h>
#include <jerry/Logger.h>

namespace jerry {
namespace engine {
namespace basic {
namespace server {

namespace {
Logger logger("jerry::engine::messaging::server::RequestContext");
}

RequestContext::RequestContext(esl::com::basic::server::RequestContext& aBaseRequestContext, Writer& aWriter, const Context& aContext)
: baseRequestContext(aBaseRequestContext),
  writer(aWriter),
  context(aContext)
{ }

esl::io::Input RequestContext::createInput(std::unique_ptr<Writer>& writer, esl::com::basic::server::requesthandler::Interface::CreateInput createInput) {
	ExceptionHandler exceptionHandler;

    /* setting 'exceptionOccured' is important to make hasRequestHandler() return true */
    bool exceptionOccured = exceptionHandler.call(
			[this, createInput]() { input = createInput(*this); });

	esl::io::Input rvInput;

    if(exceptionOccured) {
		/* **************** *
		 * Output on logger *
		 * **************** */
    	exceptionHandler.setShowException(true);
    	exceptionHandler.setShowStacktrace(true);

    	exceptionHandler.dump(logger.warn);

		rvInput = esl::io::Input(std::unique_ptr<esl::io::Writer>(writer.release()));
    }
    else if(input) {
		rvInput = esl::io::Input(std::unique_ptr<esl::io::Writer>(writer.release()));
    }

	return rvInput;
}

esl::io::Input& RequestContext::getInput() {
	return input;
}

const esl::io::Input& RequestContext::getInput() const {
	return input;
}

esl::com::basic::server::Connection& RequestContext::getConnection() const {
	return baseRequestContext.getConnection();
}

const esl::com::basic::server::Request& RequestContext::getRequest() const {
	return baseRequestContext.getRequest();
}

void RequestContext::setContext(const Context& aContext) {
	context = std::cref(aContext);
}

const Context& RequestContext::getContext() const noexcept {
	return context.get();
}

esl::object::Interface::Object* RequestContext::findObject(const std::string& id) const {
	//esl::object::Interface::Object* object = context.get().findObject<esl::object::Interface::Object>(id);
	const esl::object::ObjectContext& objectContext = context.get();
	esl::object::Interface::Object* object = objectContext.findObject<esl::object::Interface::Object>(id);

	if(object == nullptr) {
		object = baseRequestContext.findObject<esl::object::Interface::Object>(id);
	}
	return object;
}

} /* namespace server */
} /* namespace basic */
} /* namespace engine */
} /* namespace jerry */
