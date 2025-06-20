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

#include <openjerry/engine/main/Context.h>
#include <openjerry/engine/main/EntryImpl.h>
#include <openjerry/config/main/Context.h>
#include <openjerry/ExceptionHandler.h>
#include <openjerry/Logger.h>

#include <esl/monitoring/Logging.h>
#include <esl/object/InitializeContext.h>
#include <esl/object/Value.h>
#include <esl/plugin/Registry.h>
#include <esl/utility/String.h>

#include <algorithm>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <sstream>
#include <stdexcept>

namespace openjerry {
namespace engine {
namespace main {

namespace {
Logger logger("openjerry::engine::main::Context");

class ProcessLockGuard {
public:
	ProcessLockGuard(Context& aContext)
	: context(aContext)
	{
		context.processRegister(context);
	}

	~ProcessLockGuard() {
		if(locked) {
			context.processUnregister(context);
		}
	}

	void unlock() {
		if(locked) {
			context.processUnregister(context);
			locked = false;
		}
	}

private:
	Context& context;
	bool locked = true;
};

using ReturnCodeObject = esl::object::Value<int>;
} /* anonymous namespace */

std::unique_ptr<esl::object::Procedure> Context::create(const std::vector<std::pair<std::string, std::string>>& settings) {
	return std::unique_ptr<esl::object::Procedure>(new Context(settings));
}

Context::Context(const std::vector<std::pair<std::string, std::string>>& settings)
: ObjectContext(static_cast<ProcessRegistry*>(this)),
  signalManager(esl::plugin::Registry::get().findObject<esl::system::SignalManager>())
{
	bool hasCatchException = false;
	bool hasDumpException = false;
	std::string configFile;
	bool isVerbose = false;
	bool hasVerbose = false;

	for(const auto& setting : settings) {
		if(setting.first == "stop-signal") {
			stopSignals.insert(esl::system::Signal(setting.second));
		}
		else if(setting.first == "terminate-counter") {
			if(terminateCounter >= 0) {
				throw std::runtime_error("Multiple definition of attribute 'terminate-counter'");
			}
			try {
				terminateCounter = std::stoi(setting.second);
			}
			catch(...) {
				throw std::runtime_error("Invalid value \"" + setting.second + "\" for attribute 'terminate-counter'.");
			}
			if(terminateCounter < 0) {
				throw std::runtime_error("Invalid value \"" + setting.second + "\" for attribute 'terminate-counter'.");
			}
		}
		else if(setting.first == "catch-exception") {
			if(hasCatchException) {
				throw std::runtime_error("Multiple definition of attribute 'catch-exception'");
			}
			hasCatchException = true;
			if(esl::utility::String::toLower(setting.second) == "true") {
				catchException = true;
			}
			else if(esl::utility::String::toLower(setting.second) == "false") {
				catchException = false;
			}
			else {
				throw std::runtime_error("Invalid value \"" + setting.second + "\" for attribute 'catch-exception'. Allowed values are \"true\" or \"false\"");
			}
		}
		else if(setting.first == "dump-exception") {
			if(hasDumpException) {
				throw std::runtime_error("Multiple definition of attribute 'dump-exception'");
			}
			hasDumpException = true;
			if(esl::utility::String::toLower(setting.second) == "true") {
				dumpException = true;
			}
			else if(esl::utility::String::toLower(setting.second) == "false") {
				dumpException = false;
			}
			else {
				throw std::runtime_error("Invalid value \"" + setting.second + "\" for attribute 'dump-exception'. Allowed values are \"true\" or \"false\"");
			}
		}
		else if(setting.first == "exception-return-code") {
			if(hasExceptionReturnCode) {
				throw std::runtime_error("Multiple definition of attribute '" + setting.first + "'.");
			}
			hasExceptionReturnCode = true;
			try {
				exceptionReturnCode = std::stoi(setting.second);
			}
			catch(...) {
				throw std::runtime_error("Invalid value \"" + setting.second + "\" for attribute '" + setting.first + "'.");
			}
		}
		else if(setting.first == "config-file") {
			if(!configFile.empty()) {
				throw std::runtime_error("Multiple definition of attribute '" + setting.first + "'");
			}
			configFile = setting.second;
			if(configFile.empty()) {
				throw std::runtime_error("Invalid value \"" + setting.second + "\" for attribute '" + setting.first + "'.");
			}
		}
		else if(setting.first == "is-verbose") {
			if(hasVerbose) {
				throw std::runtime_error("Multiple definition of attribute '" + setting.first + "'");
			}
			isVerbose = esl::utility::String::toBool(setting.second);
			hasVerbose = true;
		}
		else {
			throw std::runtime_error("Unknown parameter key=\"" + setting.first + "\" with value=\"" + setting.second + "\"");
		}
	}

	if(!signalManager && !stopSignals.empty()) {
		logger.warn << "There are stop signals specified but there is no signal manager available. Ignoring stop signal...\n";
		stopSignals.clear();
	}

	if(!configFile.empty()) {
		std::filesystem::path serverConfigPath(configFile);
		config::main::Context mainConfig(serverConfigPath);
		if(isVerbose) {
			/* show configuration */
			mainConfig.save(std::cout);
			std::cout << "\n\n";
		}

		mainConfig.loadLibraries();

		if(isVerbose) {
			esl::plugin::Registry::get().dump(std::cout);
		}

		mainConfig.install(*this);
	}
}

void Context::addProcedure(std::unique_ptr<esl::object::Procedure> procedure) {
	entries.emplace_back(new EntryImpl(std::move(procedure)));
}

void Context::addProcedure(const std::string& refId) {
	esl::object::Procedure* procedure = findObject<esl::object::Procedure>(refId);

	if(procedure == nullptr) {
	    throw std::runtime_error("No procedure found with ref-id=\"" + refId + "\".");
	}

	entries.emplace_back(new EntryImpl(*procedure));
}

void Context::addHttpServer(std::unique_ptr<http::Server> server) {
	entries.emplace_back(new EntryImpl(std::move(server)));
}

void Context::addProcedureContext(std::unique_ptr<procedure::Context> procedureContext) {
	entries.emplace_back(new EntryImpl(std::move(procedureContext)));
}

void Context::addProcedureContext(const std::string& refId) {
	procedure::Context* context = findObject<procedure::Context>(refId);

	if(context == nullptr) {
	    throw std::runtime_error("No context found with ref-id=\"" + refId + "\".");
	}

	entries.emplace_back(new EntryImpl(*context));
}

void Context::procedureRun(esl::object::Context& objectContext) {
	logger.debug << "Starting all threads ...\n";

	/* ****************************************************************************************************** *
	 * Install stop signal handler.                                                                           *
	 * This should be done before initializing object to be able to terminate initialization (std::terminate) *
	 * ****************************************************************************************************** */
	if(!stopSignals.empty()) {
		std::thread signalThread([this](std::unique_lock<std::mutex> signalThreadRunningLock) {
			try {
				std::unique_lock<std::mutex> signalThreadLock(signalThreadMutex);
				while(true) {
					logger.trace << "SIGNAL THREAD: wait.\n";
					// sometimes infinite waiting
					// Hinweis: signalThreadMutex wird bei keinem notify_one() verwendet!
					// es sollte die zu prüfende Variable/Bedingung durch den Mutex geschützt werden:
					// Siehe:
					// - https://www.cplusplus.com/reference/condition_variable/condition_variable/
					// - https://en.cppreference.com/w/cpp/thread/condition_variable
					signalThreadCondVar.wait(signalThreadLock);
					logger.trace << "SIGNAL THREAD: wake up.\n";
					if(getProceduresRunningCount() == 0) {
						break;
					}
					logger.trace << "SIGNAL THREAD: call procedureCancel()\n";
					procedureCancel();
					if(getProceduresRunningCount() == 0) {
						break;
					}
				}
				logger.trace << "SIGNAL THREAD: quit loop.\n";
			}
			catch(...) { }
			logger.trace << "SIGNAL THREAD: done.\n";
		}, std::unique_lock<std::mutex>(signalThreadRunningMutex));
		signalThread.detach();
	}

	std::vector<esl::system::SignalManager::Handler> signalHandles;
	try {
		ProcessLockGuard processLockGuard(*this);

		for(auto signalType : stopSignals) {
			signalHandles.push_back(signalManager->createHandler(signalType, [this, signalType]() {
				if(stopSignals.count(signalType)) {
					// logger.trace << "SIGNAL FUNCTION: notify signal thread to stop signal thread\n";
					/* wake up signal thread to call "procedureCancel()" */
					signalThreadCondVar.notify_one();
				}
			}));
		}

		/* ************************* *
		 * initialize global objects *
		 * ************************* */
		logger.info << "Initialize objects ...\n";
		initializeContext();
		logger.info << "Initialization done.\n";

		logger.debug << "Start all processes...\n";

		for(auto& entry : entries) {
			{
				std::lock_guard<std::mutex> proceduresRunningLock(proceduresRunningMutex);
				if(proceduresRunningCancel) {
					break;
				}
			}
			entry->procedureRun(objectContext);
		}
		processLockGuard.unlock();

		if(getProceduresRunningCount() == 0) {
			logger.debug << "No process started.\n";
		}
		else {
			logger.debug << "All processes started.\n";

			std::unique_lock<std::mutex> processCountLock(proceduresRunningMutex);
			proceduresRunningCondVar.wait(processCountLock, [this] {
				return proceduresRunning.size() == 0;
			});

		}
	}
	catch(...) {
		if(hasExceptionReturnCode) {
			ReturnCodeObject* returnCodeObject = objectContext.findObject<ReturnCodeObject>("return-code");
			if(returnCodeObject) {
				*returnCodeObject = exceptionReturnCode;
			}
			else {
			    objectContext.addObject("return-code", std::unique_ptr<esl::object::Object>(new ReturnCodeObject(exceptionReturnCode)));
			}
		}

		if(dumpException) {
			ExceptionHandler exceptionHandler(std::current_exception());
	    	exceptionHandler.dump(std::cerr);

	    	if(esl::plugin::Registry::get().findObject<esl::monitoring::Logging>()) {
	    		std::stringstream strStream;
	    		esl::plugin::Registry::get().findObject<esl::monitoring::Logging>()->flush(&strStream);
	    		std::cerr << strStream.str();
	    	}
	    	//Logger::flush();
		}

		if(!catchException) {
			throw;
		}
	}

	/* ************************************************************************** *
	 * Remove stop signal handler and wait for signal thread has been terminated. *
	 * ************************************************************************** */
	//for(auto signalType : stopSignals) {
	//	esl::system::SignalHandler::remove(signalType, signalFunction);
	//}
	signalHandles.clear();

	if(!stopSignals.empty()) {
		/* wake up signal thread to check "getProcessCount() == 0" and following stop signal thread */
		signalThreadCondVar.notify_one();

		// check if signal thread is not running
		std::lock_guard<std::mutex> signalThreadRunningLock(signalThreadRunningMutex);
		// signal thread has been stopped
	}

	std::lock_guard<std::mutex> proceduresRunningLock(proceduresRunningMutex);
	proceduresRunningCancel = false;
}

void Context::procedureCancel() {
	if(terminateCounter == 0) {
		std::terminate();
	}
	if(terminateCounter > 0) {
		--terminateCounter;
	}

	std::set<esl::object::Procedure*> proceduresRunningCopy;
	{
		std::lock_guard<std::mutex> proceduresRunningLock(proceduresRunningMutex);
		proceduresRunningCopy = proceduresRunning;
	}
	const std::size_t proceduresTotal = proceduresRunningCopy.size();
	std::size_t proceduresCurrent = 0;

	logger.debug << "Stopping " << proceduresTotal << " procedures ...\n";
	for(const auto& procedure: proceduresRunningCopy) {
		if(procedure == this) {
			std::lock_guard<std::mutex> proceduresRunningLock(proceduresRunningMutex);
			proceduresRunningCancel = true;
		}
		else {
			if(logger.debug) {
				http::Server* httpServer = dynamic_cast<http::Server*>(procedure);

				if(httpServer) {
					logger.debug << "[" << ++proceduresCurrent << "/" << proceduresTotal << "] Stopping HTTP/HTTPS server initiated\n";
				}
				else {
					logger.debug << "[" << ++proceduresCurrent << "/" << proceduresTotal << "] Stopping procedure initiated\n";
				}
			}

			procedure->procedureCancel();
		}
	}

	logger.debug << "Stopping initiated for all procedures.\n";
}

#if 0
void Context::onSignal(const esl::utility::Signal& signal) {
	if(stopSignals.count(signal)) {
		// logger.trace << "SIGNAL FUNCTION: notify signal thread to stop signal thread\n";
		/* wake up signal thread to call "procedureCancel()" */
		signalThreadCondVar.notify_one();
	}
}
#endif

void Context::setProcessRegistry(ProcessRegistry* processRegistry) {
	ObjectContext::setProcessRegistry(processRegistry);
	for(auto& entry : entries) {
		entry->setProcessRegistry(processRegistry);
	}
}

void Context::processRegister(esl::object::Procedure& procedureRunning) {
	std::lock_guard<std::mutex> proceduresRunningLock(proceduresRunningMutex);
	proceduresRunning.insert(&procedureRunning);
}

void Context::processUnregister(esl::object::Procedure& procedureRunning) {
	{
		std::lock_guard<std::mutex> proceduresRunningLock(proceduresRunningMutex);
		if(proceduresRunning.size() == 0) {
			logger.warn << "There is no process running, but got a signal, that a process has been shutdown.\n";
		}
		else {
			proceduresRunning.erase(&procedureRunning);
		}
	}

	if(getProceduresRunningCount() == 0) {
		/* wake up this->procedureRun(...) to check "getProcessCount() == 0" and return this function */
		proceduresRunningCondVar.notify_one();

		if(!stopSignals.empty()) {
			/* wake up signal thread to check "getProcessCount() == 0" and following stop signal thread */
			signalThreadCondVar.notify_one();
		}
	}
}

void Context::addObject(const std::string& id, std::unique_ptr<esl::object::Object> object) {
	if(id.empty()) {
		//esl::processing::procedure::Interface::Procedure* procedure = dynamic_cast<esl::processing::procedure::Interface::Procedure*>(object.get());
		//if(procedure) {
		if(dynamic_cast<esl::object::Procedure*>(object.get())) {
			addProcedure(std::unique_ptr<esl::object::Procedure>(dynamic_cast<esl::object::Procedure*>(object.release())));
			return;
		}
/*
		if(dynamic_cast<procedure::Context*>(object.get())) {
			std::unique_ptr<procedure::Context> procedureContext(static_cast<procedure::Context*>(object.release()));
			procedureContext->ObjectContext::setParent(this);
			addProcedureContext(std::move(procedureContext));
			return;
		}
*/
	}
	ObjectContext::addObject(id, std::move(object));
}

void Context::initializeContext() {
	ObjectContext::initializeContext();

	// call initializeContext() of sub-context's
	for(auto& entry : entries) {
		entry->initializeContext(*this);
	}
}

void Context::dumpTree(std::size_t depth) const {
	for(std::size_t i=0; i<depth; ++i) {
		logger.info << "|   ";
	}
	logger.info << "+-> Main-Context\n";
	++depth;

	ObjectContext::dumpTree(depth);

	for(auto& entry : entries) {
		entry->dumpTree(depth);
	}
}

unsigned int Context::getProceduresRunningCount() {
	std::lock_guard<std::mutex> processCountLock(proceduresRunningMutex);
	return proceduresRunning.size();
}

} /* namespace main */
} /* namespace engine */
} /* namespace openjerry */
