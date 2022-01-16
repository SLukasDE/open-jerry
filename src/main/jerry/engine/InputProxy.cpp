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

#include <jerry/engine/InputProxy.h>
#include <jerry/ExceptionHandler.h>
#include <jerry/Logger.h>

namespace jerry {
namespace engine {

namespace {
Logger logger("jerry::engine::InputProxy");
}

esl::io::Input InputProxy::create(esl::io::Input&& input) {
	std::unique_ptr<InputProxy> inputProxy(new InputProxy(std::move(input)));
	esl::io::Consumer& consumer = inputProxy->getConsumer();
	esl::io::Writer& writer = inputProxy->getWriter();

	return esl::io::Input(std::unique_ptr<esl::object::Interface::Object>(inputProxy.release()), consumer, writer);
}

InputProxy::InputProxy(esl::io::Input&& aInput)
: input(std::move(aInput)),
  isValid(input),
  consumer(input.getConsumer(), isValid),
  writer(input.getWriter(), isValid)
{ }

esl::io::Consumer& InputProxy::getConsumer() {
	return consumer;
}

esl::io::Writer& InputProxy::getWriter() {
	return writer;
}

InputProxy::Consumer::Consumer(esl::io::Consumer& aConsumer, bool& aIsValid) :
		consumer(aConsumer), isValid(aIsValid) {
}

bool InputProxy::Consumer::consume(esl::io::Reader& reader) {
	if (isValid) {
		try {
			return consumer.consume(reader);
		} catch (...) {
			isValid = false;
			ExceptionHandler exceptionHandler(std::current_exception());
			exceptionHandler.dump(logger.warn);
		}
	}

	return false;
}

InputProxy::Writer::Writer(esl::io::Writer& aWriter, bool& aIsValid) :
		writer(aWriter), isValid(aIsValid) {
}

std::size_t InputProxy::Writer::write(const void* data, std::size_t size) {
	if (isValid) {
		try {
			return writer.write(data, size);
		} catch (...) {
			isValid = false;
			ExceptionHandler exceptionHandler(std::current_exception());
			exceptionHandler.dump(logger.warn);
		}
	}

	return npos;
}

std::size_t InputProxy::Writer::getSizeWritable() const {
	if (isValid) {
		try {
			return writer.getSizeWritable();
		} catch (...) {
			isValid = false;
			ExceptionHandler exceptionHandler(std::current_exception());
			exceptionHandler.dump(logger.warn);
		}
	}

	return npos;
}

} /* namespace engine */
} /* namespace jerry */
