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

#include <jerry/builtin/daemon/procedures/Daemon.h>
#include <jerry/Logger.h>

#include <esl/object/ObjectContext.h>
#include <esl/processing/procedure/Interface.h>
#include <esl/Stacktrace.h>

#include <stdexcept>
#include <thread>

namespace jerry {
namespace builtin {
namespace daemon {
namespace procedures {

namespace {
Logger logger("jerry::builtin::daemon::procedures::Daemon");
} /* anonymous namespace */

std::unique_ptr<esl::processing::daemon::Interface::Daemon> Daemon::create(const esl::module::Interface::Settings& settings) {
	return std::unique_ptr<esl::processing::daemon::Interface::Daemon>(new Daemon(settings));
}

Daemon::Daemon(const esl::module::Interface::Settings& settings) {
	for(const auto& setting : settings) {
		if(setting.first == "procedure-id") {
			if(setting.second.empty()) {
				throw std::runtime_error("Value \"\" of parameter 'procedure-id' is invalid");

			}
			logger.debug << "adding procedure-id '" << setting.second << "'\n";
			procedureIds.push_back(setting.second);
		}
		else {
			throw esl::addStacktrace(std::runtime_error("Unknown parameter key=\"" + setting.first + "\" with value=\"" + setting.second + "\""));
		}
	}
}

Daemon::~Daemon() {
	wait(0);
}

void Daemon::initializeContext(esl::object::Interface::ObjectContext& objectContext) {
	logger.debug << "Initialize: Lookup " << procedureIds.size() << " procedure-ids\n";
	for(const auto& procedureId : procedureIds) {
		esl::processing::procedure::Interface::Procedure* procedure = objectContext.findObject<esl::processing::procedure::Interface::Procedure>(procedureId);
		if(procedure == nullptr) {
			throw esl::addStacktrace(std::runtime_error("Cannot find procedure with id \"" + procedureId + "\""));
		}
		procedures.push_back(procedure);
	}
	logger.debug << procedures.size() << " procedures\n";
}

bool Daemon::start(std::function<void()> aOnReleasedHandler) {
	{
		std::lock_guard<std::mutex> stateLock(stateMutex);
		if(state != stopped) {
			return false;
		}
		state = started;
	}

	onReleasedHandler = aOnReleasedHandler;

	std::thread runThread([this]{
		run();
	});
	runThread.detach();

	return true;
}

void Daemon::release() {
	std::lock_guard<std::mutex> stateLock(stateMutex);
	if(state == started) {
		state = stopping;
	}
	if(state == stopped && onReleasedHandler) {
		onReleasedHandler();
		onReleasedHandler = nullptr;
	}
}

bool Daemon::wait(std::uint32_t ms) {
	{
		std::lock_guard<std::mutex> stateLock(stateMutex);
		if(state == stopped) {
			return true;
		}
	}

	{
		std::unique_lock<std::mutex> stateNotifyLock(stateNotifyMutex);
		if(ms == 0) {
			stateNotifyCondVar.wait(stateNotifyLock, [this] {
					std::lock_guard<std::mutex> stateLock(stateMutex);
					return state == stopped;
			});
			return true;
		}
		else {
			return stateNotifyCondVar.wait_for(stateNotifyLock, std::chrono::milliseconds(ms), [this] {
					std::lock_guard<std::mutex> stateLock(stateMutex);
					return state == stopped;
			});
		}
	}
	return true;
}

void Daemon::run() {
	esl::object::ObjectContext objectContext;

	try {
		for(auto procedure : procedures) {
			{
				std::lock_guard<std::mutex> stateLock(stateMutex);

				if(state != started) {
					break;
				}
			}
			procedure->procedureRun(objectContext);
		}
	}
	catch(...) { }
	{
		std::lock_guard<std::mutex> stateLock(stateMutex);
		state = stopped;
		if(onReleasedHandler) {
			onReleasedHandler();
			onReleasedHandler = nullptr;
		}
	}
	stateNotifyCondVar.notify_all();
}

} /* namespace procedures */
} /* namespace daemon */
} /* namespace builtin */
} /* namespace jerry */
