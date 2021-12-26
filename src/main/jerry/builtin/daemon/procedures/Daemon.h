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

#ifndef JERRY_BUILTIN_DAEMON_PROCEDURES_DAEMON_H_
#define JERRY_BUILTIN_DAEMON_PROCEDURES_DAEMON_H_

#include <esl/processing/daemon/Interface.h>
#include <esl/processing/procedure/Interface.h>
#include <esl/object/Interface.h>
#include <esl/object/InitializeContext.h>
#include <esl/module/Interface.h>

#include <string>
#include <vector>
#include <memory>
#include <mutex>
#include <condition_variable>
#include <functional>

namespace jerry {
namespace builtin {
namespace daemon {
namespace procedures {

class Daemon final : public virtual esl::processing::daemon::Interface::Daemon, public esl::object::InitializeContext {
public:
	static inline const char* getImplementation() {
		return "jerry/builtin/daemon/procedures";
	}

	static std::unique_ptr<esl::processing::daemon::Interface::Daemon> create(const esl::module::Interface::Settings& settings);

	Daemon(const esl::module::Interface::Settings& settings);
	~Daemon();

	void initializeContext(esl::object::Interface::ObjectContext& objectContext) override;

	bool start(std::function<void()> onReleasedHandler) override;
	void release() override;
	bool wait(std::uint32_t ms) override;

private:
	std::vector<std::string> procedureIds;
	std::vector<esl::processing::procedure::Interface::Procedure*> procedures;

	std::function<void()> onReleasedHandler;

	std::mutex stateMutex;
	enum {
		stopped,
		started,
		stopping
	} state = stopped;

	std::mutex stateNotifyMutex;
	std::condition_variable stateNotifyCondVar;

	void run();
};

} /* namespace procedures */
} /* namespace daemon */
} /* namespace builtin */
} /* namespace jerry */

#endif /* JERRY_BUILTIN_DAEMON_PROCEDURES_DAEMON_H_ */
