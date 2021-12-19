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

#ifndef JERRY_ENGINE_INPUTPROXY_H_
#define JERRY_ENGINE_INPUTPROXY_H_

#include <esl/object/Interface.h>
#include <esl/io/Input.h>
#include <esl/io/Consumer.h>
#include <esl/io/Writer.h>
#include <esl/io/Reader.h>

#include <string>
#include <memory>

namespace jerry {
namespace engine {

class InputProxy : public esl::object::Interface::Object {
public:
	static esl::io::Input create(esl::io::Input&& input);

protected:
	InputProxy(esl::io::Input&& input);

	esl::io::Consumer& getConsumer();
	esl::io::Writer& getWriter();

private:
	esl::io::Input input;
	bool isValid;

	class Consumer : public esl::io::Consumer {
	public:
		Consumer(esl::io::Consumer& consumer, bool& isValid);
		bool consume(esl::io::Reader& reader) override;

	private:
		esl::io::Consumer& consumer;
		bool& isValid;
	} consumer;

	class Writer : public esl::io::Writer {
	public:
		Writer(esl::io::Writer& writer, bool& isValid);

		std::size_t write(const void* data, std::size_t size) override;
		std::size_t getSizeWritable() const override;

	private:
		esl::io::Writer& writer;
		bool& isValid;
	} writer;
};

} /* namespace engine */
} /* namespace jerry */

#endif /* JERRY_ENGINE_INPUTPROXY_H_ */
