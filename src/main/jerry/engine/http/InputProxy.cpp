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

#include <jerry/engine/http/InputProxy.h>
#include <jerry/engine/http/ExceptionHandler.h>
#include <jerry/Logger.h>

namespace jerry {
namespace engine {
namespace http {


namespace {
Logger logger("jerry::engine::http::InputProxy");
}

esl::io::Input InputProxy::create(esl::io::Input&& input, std::unique_ptr<RequestContext> requestContext) {
	std::unique_ptr<InputProxy> inputProxy(new InputProxy(std::move(input), std::move(requestContext)));
	esl::io::Consumer& consumer = inputProxy->getConsumer();
	esl::io::Writer& writer = inputProxy->getWriter();
	return esl::io::Input(std::unique_ptr<esl::object::Interface::Object>(inputProxy.release()), consumer, writer);
}

InputProxy::InputProxy(esl::io::Input&& aInput, std::unique_ptr<RequestContext> aRequestContext)
: input(std::move(aInput)),
  isValid(input),
  requestContext(std::move(aRequestContext)),
  consumer(input.getConsumer(), isValid, *requestContext),
  writer(input.getWriter(), isValid, *requestContext)
{ }

esl::io::Consumer& InputProxy::getConsumer() {
	return consumer;
}

esl::io::Writer& InputProxy::getWriter() {
	return writer;
}

InputProxy::Consumer::Consumer(esl::io::Consumer& aConsumer, bool& aIsValid, RequestContext& aRequestContext)
: consumer(aConsumer),
  isValid(aIsValid),
  requestContext(aRequestContext)
{ }

bool InputProxy::Consumer::consume(esl::io::Reader& reader) {
	if(isValid) {
		try {
			return consumer.consume(reader);
		}
		catch(...) {
			isValid = false;
			ExceptionHandler exceptionHandler(std::current_exception());
	    	exceptionHandler.dump(logger.warn);
			exceptionHandler.dumpHttp(requestContext.getConnection(), requestContext.getErrorHandlingContext(), requestContext.getHeadersContext());
		}
	}

	return false;
}

InputProxy::Writer::Writer(esl::io::Writer& aWriter, bool& aIsValid, RequestContext& aRequestContext)
: writer(aWriter),
  isValid(aIsValid),
  requestContext(aRequestContext)
{ }

std::size_t InputProxy::Writer::write(const void* data, std::size_t size) {
	if(isValid) {
		try {
	    	return writer.write(data, size);
		}
		catch(...) {
			isValid = false;
			ExceptionHandler exceptionHandler(std::current_exception());
	    	exceptionHandler.dump(logger.warn);
			exceptionHandler.dumpHttp(requestContext.getConnection(), requestContext.getErrorHandlingContext(), requestContext.getHeadersContext());
		}
	}

	return npos;
}

std::size_t InputProxy::Writer::getSizeWritable() const {
	if(isValid) {
		try {
			return writer.getSizeWritable();
		}
		catch(...) {
			isValid = false;
			ExceptionHandler exceptionHandler(std::current_exception());
	    	exceptionHandler.dump(logger.warn);
			exceptionHandler.dumpHttp(requestContext.getConnection(), requestContext.getErrorHandlingContext(), requestContext.getHeadersContext());
		}
	}

	return npos;
}


} /* namespace http */
} /* namespace engine */
} /* namespace jerry */
