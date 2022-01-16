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

#include <jerry/builtin/daemon/procedure/Daemon.h>
#include <jerry/builtin/daemon/procedure/ObjectContext.h>
#include <jerry/ExceptionHandler.h>
#include <jerry/Logger.h>

#include <esl/processing/procedure/Interface.h>
#include <esl/Stacktrace.h>

#include <stdexcept>
#include <thread>

namespace jerry {
namespace builtin {
namespace daemon {
namespace procedure {

namespace {
Logger logger("jerry::builtin::daemon::procedure::Daemon");
} /* anonymous namespace */

std::unique_ptr<esl::processing::daemon::Interface::Daemon> Daemon::create(const esl::module::Interface::Settings& settings) {
	return std::unique_ptr<esl::processing::daemon::Interface::Daemon>(new Daemon(settings));
}

Daemon::Daemon(const esl::module::Interface::Settings& settings) {
	for(const auto& setting : settings) {
		if(setting.first == "procedure-id") {
			if(!procedureId.empty()) {
				throw std::runtime_error("Multiple definition of attribute 'procedure-id'.");
			}
			procedureId = setting.second;
			if(procedureId.empty()) {
				throw std::runtime_error("Invalid value \"\" for attribute 'procedure-id'.");
			}
		}
		else {
			throw std::runtime_error("Unknown parameter key=\"" + setting.first + "\" with value=\"" + setting.second + "\"");
		}
	}

	if(procedureId.empty()) {
		throw std::runtime_error("Definition of attribute 'procedure-id' is missing.");
	}
}

Daemon::~Daemon() {
	wait(0);
}

void Daemon::initializeContext(esl::object::ObjectContext& objectContext) {
	logger.debug << "Initialize: Lookup procedureId \"" << procedureId << "\"\n";

	procedure = objectContext.findObject<esl::processing::procedure::Interface::Procedure>(procedureId);
	if(procedure == nullptr) {
		throw std::runtime_error("Cannot find procedure with id \"" + procedureId + "\"");
	}
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
	/* cancel current procedure */
	if(procedure) {
		procedure->procedureCancel();
	}

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
	ObjectContext objectContext;

	try {
		bool isStopped;
		{
			std::lock_guard<std::mutex> stateLock(stateMutex);

			isStopped = (state != started);
		}
		if(!isStopped) {
			procedure->procedureRun(objectContext);
		}
	}
	catch(...) {
		ExceptionHandler exceptionHandler(std::current_exception());
    	exceptionHandler.dump(logger.error);
	}

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

} /* namespace procedure */
} /* namespace daemon */
} /* namespace builtin */
} /* namespace jerry */
