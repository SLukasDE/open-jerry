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

#ifndef JERRY_BUILTIN_MESSAGING_ECHO_SETTINGS_H_
#define JERRY_BUILTIN_MESSAGING_ECHO_SETTINGS_H_

#include <esl/object/Settings.h>
#include <esl/object/InitializeContext.h>
#include <esl/object/Interface.h>
#include <esl/messaging/Producer.h>
#include <esl/messaging/Interface.h>

#include <string>
#include <memory>

namespace jerry {
namespace builtin {
namespace messaging {
namespace echo {

class Settings : public esl::object::Settings, public esl::object::InitializeContext {
public:
	static std::unique_ptr<esl::object::Interface::Object> create();

	void addSetting(const std::string& key, const std::string& value) override;
	void initializeContext(esl::object::ObjectContext& objectContext) override;

	std::unique_ptr<esl::messaging::Producer> createProducer();
	unsigned long getMSDelay() const;

private:
	std::string queue;
	unsigned long msDelay = 0;
	esl::messaging::Interface::ProducerFactory* producerFactory = nullptr;;
};

} /* namespace echo */
} /* namespace messaging */
} /* namespace builtin */
} /* namespace jerry */

#endif /* JERRY_BUILTIN_MESSAGING_ECHO_SETTINGS_H_ */
