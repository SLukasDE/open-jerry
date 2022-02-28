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

#include <jerry/Procedure.h>
#include <jerry/config/Engine.h>
#include <jerry/engine/http/Socket.h>
#include <jerry/engine/basic/Socket.h>
#include <jerry/ExceptionHandler.h>
#include <jerry/Module.h>
#include <jerry/Logger.h>

#include <esl/Module.h>
#include <esl/object/Value.h>
#include <esl/logging/appender/Appender.h>
#include <esl/logging/layout/Layout.h>
#include <esl/logging/Logger.h>
#include <esl/processing/daemon/Interface.h>
#include <esl/system/SignalHandler.h>

#include <functional>
#include <stdexcept>
#include <sstream>
#include <iostream>

namespace jerry {

namespace {
using ReturnCodeObject = esl::object::Value<int>;

std::unique_ptr<esl::logging::appender::Interface::Appender> appenderCoutStream;
std::unique_ptr<esl::logging::appender::Interface::Appender> appenderMemBuffer;
std::unique_ptr<esl::logging::layout::Interface::Layout> layout;

Logger logger("jerry::Procedure");
} /* anonymous namespace */

std::unique_ptr<esl::processing::procedure::Interface::Procedure> Procedure::create(const esl::module::Interface::Settings& settings) {
	return std::unique_ptr<esl::processing::procedure::Interface::Procedure>(new Procedure(settings));
}

Procedure::Procedure(const esl::module::Interface::Settings& settings) {
	bool hasVerbose = false;

	for(const auto& setting : settings) {
		if(setting.first == "config") {
			if(!config.empty()) {
				throw std::runtime_error("Multiple definition of attribute 'config'");
			}
			if(!configFile.empty()) {
				throw std::runtime_error("Definition of attribute 'config' is not allowed together with attribute 'config-file'");
			}
			config = setting.second;
			if(config.empty()) {
				throw std::runtime_error("Invalid value \"\" for attribute 'config'");
			}
		}
		else if(setting.first == "config-file") {
			if(!configFile.empty()) {
				throw std::runtime_error("Multiple definition of attribute 'config-file'");
			}
			if(!config.empty()) {
				throw std::runtime_error("Definition of attribute 'config-file' is not allowed together with attribute 'config'");
			}
			configFile = setting.second;
			if(configFile.empty()) {
				throw std::runtime_error("Invalid value \"\" for attribute 'config-file'");
			}
		}
		else if(setting.first == "stop-signal") {
			esl::system::Interface::SignalType signalType = esl::system::Interface::SignalType::unknown;
			if(setting.second == "interrupt") {
				signalType = esl::system::Interface::SignalType::interrupt;
			}
			else if(setting.second == "terminate") {
				signalType = esl::system::Interface::SignalType::terminate;
			}
			else if(setting.second == "pipe") {
				signalType = esl::system::Interface::SignalType::pipe;
			}
			else {
				throw std::runtime_error("Invalid value \"" + setting.second + "\" for attribute 'stop-signal'");
			}
			stopSignals.insert(signalType);
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
		else if(setting.first == "verbose") {
			if(hasVerbose) {
				throw std::runtime_error("Multiple definition of attribute 'verbose'");
			}
			hasVerbose = true;
			if(setting.second == "true") {
				verbose = true;
			}
			else if(setting.second == "false") {
				verbose = false;
			}
			else {
				throw std::runtime_error("Invalid value \"" + setting.second + "\" for attribute 'verbose'. Allowed values are \"true\" or \"false\"");
			}
		}
		else {
			throw std::runtime_error("Unknown parameter key=\"" + setting.first + "\" with value=\"" + setting.second + "\"");
		}
	}

	if(config.empty() && configFile.empty()) {
		throw std::runtime_error("Missing attribute 'config' or 'config-file'");
	}

	if(!configFile.empty()) {
		try {
			configEngine.reset(new config::Engine(configFile));
		}
		catch(...) {
			ExceptionHandler exceptionHandler(std::current_exception());
	    	exceptionHandler.dump(std::cerr);
	    	throw std::runtime_error("Failed to load config-file \"" + configFile + "\"");
		}
	}
	else if(!config.empty()) {
		try {
			configEngine.reset(new config::Engine(config.c_str(), config.size()));
		}
		catch(...) {
			ExceptionHandler exceptionHandler(std::current_exception());
	    	exceptionHandler.dump(std::cerr);
	    	throw std::runtime_error("Failed to parse config");
		}
	}
	else {
		throw std::runtime_error("Initialization failed: config or config-file is missing");
	}

	if(verbose) {
		/* show configuration */
		configEngine->save(std::cout);
		std::cout << "\n\n";
	}

	try {
		configEngine->loadLibraries();

		if(verbose) {
			/* show loaded modules and interfaces */
			std::cout << "Interfaces:\n";
			std::cout << "-----------\n";
			for(const auto& interface : esl::getModule().getMetaInterfaces()) {
				std::cout << "  module:         \"" << interface.module << "\"\n";
				std::cout << "  type:           \"" << interface.type << "\"\n";
				std::cout << "  implementation: \"" << interface.implementation << "\"\n";
				std::cout << "  apiVersion:     \"" << interface.apiVersion << "\"\n";
				std::cout << "\n";
			}
			std::cout << "\n\n";
		}

		layout = configEngine->installLogging();

		appenderCoutStream.reset(new esl::logging::appender::Appender({
			{"trace", "out"},
			{"info", "out"},
			{"debug", "out"},
			{"warn", "err"},
			{"error", "err"}}, "eslx/ostream"));

		appenderMemBuffer.reset(new esl::logging::appender::Appender({
			{"max-lines", "100"}}, "eslx/membuffer"));

		/* *********************** *
		 * set layout to appenders *
		 * *********************** */

	    appenderCoutStream->setRecordLevel();
	    appenderCoutStream->setLayout(layout.get());
	    esl::logging::addAppender(*appenderCoutStream);

	    /* MemBuffer appender just writes output to a buffer of a fixed number of lines.
	     * If number of columns is specified as well the whole memory is allocated at initialization time.
	     */
	    appenderMemBuffer->setRecordLevel(esl::logging::appender::Interface::Appender::RecordLevel::ALL);
	    appenderMemBuffer->setLayout(layout.get());
	    esl::logging::addAppender(*appenderMemBuffer);
	}
	catch(...) {
		ExceptionHandler exceptionHandler(std::current_exception());
    	exceptionHandler.dump(std::cerr);
    	throw std::runtime_error("Failed to create jerry procedure");
	}
}

void Procedure::procedureRun(esl::object::ObjectContext& objectContext) {
	int returnCode = 0;

	if(!configEngine) {
		throw std::runtime_error("Initialization failed: empty config");
	}

	jEngine.reset(new engine::Engine([this]() {
		processRegister();
	}, [this]() {
		processUnregister();
	}));
	configEngine->installEngine(*jEngine);

	if(verbose) {
		/* show configuration file */
		jEngine->dumpTree(0);
		std::cout << "\n\n";
	}

	if(getProcessCount() > 0) {
		logger.warn << "procedureRun called, but procedure is already running.\n";
		return;
	}



	/* ******************* *
	 * set batch procedure *
	 * ******************* */
	{
		esl::processing::procedure::Interface::Procedure* batchProcedure = objectContext.findObject<esl::processing::procedure::Interface::Procedure>("");
		if(batchProcedure) {
			jEngine->setBatchProcedure(batchProcedure);
		}
	}

	/* ****************************************************************************************************** *
	 * Install stop signal handler.                                                                           *
	 * This should be done before initializing object to be able to terminate initialization (std::terminate) *
	 * ****************************************************************************************************** */
	std::function<void()> signalFunction = [this]() {
		logger.trace << "SIGNAL FUNCTION: notify signal thread to stop signal thread\n";
		/* wake up signal thread to call "procedureCancel()" */
		signalThreadCondVar.notify_one();
	};
	if(!stopSignals.empty()) {
		std::thread signalThread([this](std::unique_lock<std::mutex> signalThreadRunningLock) {
			try {
			    std::unique_lock<std::mutex> signalThreadLock(signalThreadMutex);
				while(true) {
	    			logger.trace << "SIGNAL THREAD: wait.\n";
				    signalThreadCondVar.wait(signalThreadLock);
	    			logger.trace << "SIGNAL THREAD: wake up.\n";
		            if(getProcessCount() == 0) {
		            	break;
		            }
	    			logger.trace << "SIGNAL THREAD: call procedureCancel()\n";
		    		procedureCancel();
		            if(getProcessCount() == 0) {
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
	for(auto signalType : stopSignals) {
		esl::system::SignalHandler::install(signalType, signalFunction);
	}

	try {
		/* ************************* *
		 * initialize global objects *
		 * ************************* */
		logger.info << "Initialize objects ...\n";
		jEngine->initializeContext();
		logger.info << "Initialization done.\n";

		jEngine->startServers();

		if(jEngine->getBatchProcedure()) {
			processRegister();
			logger.debug << "Batch process started\n";
		}

		if(getProcessCount() == 0) {
			logger.debug << "No process started.\n";
		}
		else {
			logger.debug << "All processes started.\n";

			if(jEngine->getBatchProcedure()) {
				//jEngine->getBatchProcedure()->procedureRun(*jEngine);
				try {
					jEngine->getBatchProcedure()->procedureRun(objectContext);
				}
				catch(...) {
					processUnregister();
					if(getProcessCount() > 0) {
						logger.trace << "PROC RUN: wait for servers ...\n";
						std::unique_lock<std::mutex> processCountLock(processCountMutex);
						processCountCondVar.wait(processCountLock, [this] {
							return processCount == 0;
						});
						logger.trace << "PROC RUN: wait for servers done.\n";
					}
					throw;
				}
				processUnregister();
			}

			if(getProcessCount() > 0) {
				logger.trace << "PROC RUN: wait for servers ...\n";
				std::unique_lock<std::mutex> processCountLock(processCountMutex);
				processCountCondVar.wait(processCountLock, [this] {
					return processCount == 0;
				});
				logger.trace << "PROC RUN: wait for servers done.\n";
			}
		}
	}
	catch(...) {
		ExceptionHandler exceptionHandler(std::current_exception());
    	exceptionHandler.dump(std::cerr);

		flushLogAppender("eslx/ostream", *appenderCoutStream);
		flushLogAppender("eslx/membuffer", *appenderMemBuffer);

		returnCode = -1;
	}

	/* ************************************************************************** *
	 * Remove stop signal handler and wait for signal thread has been terminated. *
	 * ************************************************************************** */
	for(auto signalType : stopSignals) {
		esl::system::SignalHandler::remove(signalType, signalFunction);
	}
	if(!stopSignals.empty()) {
		logger.trace << "PROC RUN: notify signal thread to stop signal thread\n";
		/* wake up signal thread to check "getProcessCount() == 0" and following stop signal thread */
		signalThreadCondVar.notify_one();

		logger.trace << "PROC RUN: check if signal thread is not running\n";
		std::lock_guard<std::mutex> signalThreadRunningLock(signalThreadRunningMutex);
		logger.trace << "PROC RUN: check successful: signal thread has been stopped\n";
	}

	ReturnCodeObject* returnCodeObject = objectContext.findObject<ReturnCodeObject>("return-code");
	if(!returnCodeObject) {
	    objectContext.addObject("return-code", std::unique_ptr<esl::object::Interface::Object>(new ReturnCodeObject(returnCode)));
	}
	else if(returnCode != 0) {
		*returnCodeObject = returnCode;
	}
	logger.trace << "PROC RUN: done\n";
}

void Procedure::procedureCancel() {
	if(!jEngine) {
		return;
	}

	if(terminateCounter == 0) {
		std::terminate();
	}
	if(terminateCounter > 0) {
		--terminateCounter;
	}

	logger.info << "stopping engine\n";

	jEngine->stopServers();

	if(jEngine->getBatchProcedure()) {
		jEngine->getBatchProcedure()->procedureCancel();
		logger.debug << "Stopping batch process initiated\n";
	}
}

unsigned int Procedure::getProcessCount() {
	std::lock_guard<std::mutex> processCountLock(processCountMutex);
	return processCount;
}

void Procedure::processRegister() {
	std::lock_guard<std::mutex> processCountLock(processCountMutex);
	++processCount;
}

void Procedure::processUnregister() {
	logger.trace << "PROC UNREG: Got a signal, that a process has been shutdown.\n";
	{
		std::lock_guard<std::mutex> processCountLock(processCountMutex);
		if(processCount == 0) {
			logger.warn << "PROC UNREG: There is no process running, but got a signal, that a process has been shutdown.\n";
		}
		else {
			--processCount;
		}
		logger.trace << "PROC UNREG: " <<  processCount << " processes running\n";
	}

	if(getProcessCount() == 0) {
		logger.trace << "PROC UNREG: notify this->procedureRun()\n";
		/* wake up this->procedureRun(...) to check "getProcessCount() == 0" and return this function */
		processCountCondVar.notify_one();

		if(!stopSignals.empty()) {
			logger.trace << "PROC UNREG: notify signal thread to stop signal thread\n";
			/* wake up signal thread to check "getProcessCount() == 0" and following stop signal thread */
			signalThreadCondVar.notify_one();
		}
	}
}

void Procedure::flushLogAppender(const std::string& id, esl::logging::appender::Interface::Appender& appender) {
	std::stringstream strStream;
	appender.flush();
	appender.flush(strStream);
	if(!strStream.str().empty()) {
		std::cerr << "\n\nFlush log messages " << id << ":\n";
		std::cerr << strStream.str();
	}
}

} /* namespace jerry */
