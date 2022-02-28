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

#include <jerry/engine/Daemon.h>
#include <jerry/ObjectContext.h>
#include <jerry/ExceptionHandler.h>
#include <jerry/Logger.h>

#include <esl/object/InitializeContext.h>
#include <esl/Module.h>

#include <thread>
#include <stdexcept>

namespace jerry {
namespace engine {

namespace {
Logger logger("jerry::Daemon");
} /* anonymous namespace */

Daemon::Daemon(std::unique_ptr<esl::processing::procedure::Interface::Procedure> aProcedure)
: procedure(std::move(aProcedure))
{
	if(!procedure) {
		throw std::runtime_error("Cannot create a daemon with an empty procedure.");
	}
}

Daemon::~Daemon() {
	runningMutex.lock();
}

void Daemon::initializeContext(esl::object::ObjectContext& objectContext) {
	esl::object::InitializeContext* initializeContext = dynamic_cast<esl::object::InitializeContext*>(procedure.get());
	if(initializeContext) {
		initializeContext->initializeContext(objectContext);
	}
}


void Daemon::start(std::function<void()> aOnReleasedHandler) {
	if(runningMutex.try_lock() == false) {
		return;
	}

	onReleasedHandler = aOnReleasedHandler;

	std::thread runThread([this] {
		try {
			ObjectContext objectContext;
			if(procedure) {
				procedure->procedureRun(objectContext);
			}
		}
		catch(...) {
			ExceptionHandler exceptionHandler(std::current_exception());
	    	exceptionHandler.dump(logger.error);
		}

		if(onReleasedHandler) {
			try {
				onReleasedHandler();
			}
			catch(...) {
				ExceptionHandler exceptionHandler(std::current_exception());
		    	exceptionHandler.dump(logger.error);
			}
		}
		runningMutex.unlock();
	});

	runThread.detach();
}

void Daemon::release() {
	/* cancel procedure */
	if(procedure) {
		procedure->procedureCancel();
	}
}

} /* namespace engine */
} /* namespace jerry */
