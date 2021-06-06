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

#ifndef JERRY_BUILTIN_BASIC_ECHO_SETTINGS_H_
#define JERRY_BUILTIN_BASIC_ECHO_SETTINGS_H_

#include <esl/object/Settings.h>
#include <esl/object/InitializeContext.h>
#include <esl/object/Interface.h>
#include <esl/com/basic/broker/Interface.h>
#include <esl/com/basic/client/Interface.h>

#include <string>
#include <set>
#include <vector>
#include <utility>
#include <memory>

namespace jerry {
namespace builtin {
namespace basic {
namespace echo {

class Settings : public esl::object::Settings, public esl::object::InitializeContext {
public:
	static std::unique_ptr<esl::object::Interface::Object> create();

	void addSetting(const std::string& key, const std::string& value) override;
	void initializeContext(esl::object::ObjectContext& objectContext) override;

	const std::set<std::string>& getNotifiers() const noexcept;

	std::unique_ptr<esl::com::basic::client::Interface::Connection> createConnection();
	unsigned long getMSDelay() const;

private:
	std::set<std::string> notifiers;
	unsigned long msDelay = 0;
	std::string outputRefId;
	std::vector<std::pair<std::string, std::string>> outputParameters;
	esl::com::basic::broker::Interface::Client* client = nullptr;
};

} /* namespace echo */
} /* namespace basic */
} /* namespace builtin */
} /* namespace jerry */

#endif /* JERRY_BUILTIN_BASIC_ECHO_SETTINGS_H_ */
