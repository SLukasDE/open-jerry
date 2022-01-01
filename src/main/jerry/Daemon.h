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

#ifndef JERRY_DAEMON_H_
#define JERRY_DAEMON_H_

#include <jerry/engine/Engine.h>

#include <esl/processing/daemon/Interface.h>
#include <esl/utility/MessageTimer.h>

#include <functional>
#include <cstdint>
#include <mutex>
#include <condition_variable>
#include <string>

namespace jerry {

class Daemon : public esl::processing::daemon::Interface::Daemon {
public:
	Daemon();
	~Daemon();

	bool setupXML(const std::string& configFile, bool dumpXML);

	bool run();
	engine::Engine& getEngine();

	bool start(std::function<void()> onReleasedHandler) override;
	void release() override;
	bool wait(std::uint32_t ms) override;

private:
	enum State {
		stopped,
		started,
		stopping
	};

	engine::Engine jEngine;
	bool isInitialized = false;
    esl::utility::MessageTimer<int, State> messageTimer;
    int stopSignalCounter = -1;
    std::function<void()> onReleasedHandler;

	std::mutex stateMutex;
	State state = stopped;
	unsigned int runningStates = 0;

	std::mutex stateNotifyMutex;
	std::condition_variable stateNotifyCondVar;

	void onReleased();
	void stopSignal();
	void initialize();
};

} /* namespace jerry */

#endif /* JERRY_DAEMON_H_ */
