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

#include <jerry/engine/main/Context.h>
#include <jerry/engine/main/EntryImpl.h>
#include <jerry/config/main/Context.h>
#include <jerry/ExceptionHandler.h>
#include <jerry/Logger.h>
#include <jerry/Module.h>

#include <esl/Module.h>
#include <esl/Stacktrace.h>
#include <esl/object/InitializeContext.h>
#include <esl/object/Value.h>
#include <esl/logging/appender/Appender.h>
#include <esl/logging/layout/Layout.h>
#include <esl/system/SignalHandler.h>
#include <esl/utility/String.h>

#include <set>
#include <fstream>
#include <vector>
#include <algorithm>
#include <stdexcept>
#include <iostream>
#include <sstream>

namespace jerry {
namespace engine {
namespace main {

namespace {
Logger logger("jerry::engine::main::Context");

std::unique_ptr<esl::logging::appender::Interface::Appender> appenderCoutStream;
std::unique_ptr<esl::logging::appender::Interface::Appender> appenderMemBuffer;
std::unique_ptr<esl::logging::layout::Interface::Layout> layout;

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

std::unique_ptr<esl::processing::procedure::Interface::Procedure> Context::create(const std::vector<std::pair<std::string, std::string>>& settings) {
	return std::unique_ptr<esl::processing::procedure::Interface::Procedure>(new Context(settings));
}

Context::Context(const std::vector<std::pair<std::string, std::string>>& settings)
: ObjectContext(static_cast<ProcessRegistry&>(*this))
{
	bool hasVerbose = false;
	bool hasCatchException = false;

	for(const auto& setting : settings) {
		if(setting.first == "config-data") {
			if(!configData.empty()) {
				throw std::runtime_error("Multiple definition of attribute 'config-data'");
			}
			if(!configFile.empty()) {
				throw std::runtime_error("Definition of attribute 'config-data' is not allowed together with attribute 'config-file'");
			}
			configData = setting.second;
			if(configData.empty()) {
				throw std::runtime_error("Invalid value \"\" for attribute 'config-data'");
			}
		}
		else if(setting.first == "config-file") {
			if(!configFile.empty()) {
				throw std::runtime_error("Multiple definition of attribute 'config-file'");
			}
			if(!configData.empty()) {
				throw std::runtime_error("Definition of attribute 'config-file' is not allowed together with attribute 'config-data'");
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
			if(esl::utility::String::toLower(setting.second) == "true") {
				verbose = true;
			}
			else if(esl::utility::String::toLower(setting.second) == "false") {
				verbose = false;
			}
			else {
				throw std::runtime_error("Invalid value \"" + setting.second + "\" for attribute 'verbose'. Allowed values are \"true\" or \"false\"");
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
		else if(setting.first == "exception-return-code") {
			if(hasExceptionReturnCode) {
				throw std::runtime_error("Multiple definition of attribute 'exception-return-code'");
			}
			hasExceptionReturnCode = true;
			try {
				exceptionReturnCode = std::stoi(setting.second);
			}
			catch(...) {
				throw std::runtime_error("Invalid value \"" + setting.second + "\" for attribute 'exception-return-code'.");
			}
		}
		else {
			throw std::runtime_error("Unknown parameter key=\"" + setting.first + "\" with value=\"" + setting.second + "\"");
		}
	}

	std::unique_ptr<config::main::Context> config;
	if(!configFile.empty()) {
		try {
			config.reset(new config::main::Context(configFile));
		}
		catch(...) {
			ExceptionHandler exceptionHandler(std::current_exception());
	    	exceptionHandler.dump(std::cerr);
	    	throw std::runtime_error("Failed to load config-file \"" + configFile + "\"");
		}
	}
	else if(!configData.empty()) {
		try {
			config.reset(new config::main::Context(configData.c_str(), configData.size()));
		}
		catch(...) {
			ExceptionHandler exceptionHandler(std::current_exception());
	    	exceptionHandler.dump(std::cerr);
	    	throw std::runtime_error("Failed to parse config");
		}
	}
	else {
		throw std::runtime_error("Missing attribute 'config-data' or 'config-file'");
	}

	if(verbose) {
		/* show configuration */
		config->save(std::cout);
		std::cout << "\n\n";
	}

	try {
		config->loadLibraries();

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

		layout = config->installLogging();

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


		config->install(*this);
	}
	catch(...) {
		ExceptionHandler exceptionHandler(std::current_exception());
    	exceptionHandler.dump(std::cerr);
    	throw std::runtime_error("Failed to create jerry procedure");
	}
}

void Context::addCertificate(const std::string& hostname, std::vector<unsigned char> key, std::vector<unsigned char> certificate) {
	std::pair<std::vector<unsigned char>, std::vector<unsigned char>>& certPair = certsByHostname[hostname];
	certPair.first = std::move(certificate);
	certPair.second = std::move(key);
}

void Context::addCertificate(const std::string& hostname, const std::string& keyFile, const std::string& certificateFile) {
	std::vector<unsigned char> key;
	std::vector<unsigned char> certificate;

	if(!keyFile.empty()) {
		std::ifstream ifStream(keyFile, std::ios::binary );
		if(!ifStream.good()) {
			throw std::runtime_error("Cannot open key file \"" + keyFile + "\"");
		}
	    key = std::vector<unsigned char>(std::istreambuf_iterator<char>(ifStream), {});
	}

	if(!certificateFile.empty()) {
		std::ifstream ifStream(certificateFile, std::ios::binary );
		if(!ifStream.good()) {
			throw std::runtime_error("Cannot open certificate file \"" + certificateFile + "\"");
		}
		certificate = std::vector<unsigned char>(std::istreambuf_iterator<char>(ifStream), {});
	}

    addCertificate(hostname, std::move(key), std::move(certificate));
}

const std::map<std::string, std::pair<std::vector<unsigned char>, std::vector<unsigned char>>>& Context::getCertificates() const noexcept {
	return certsByHostname;
}

const std::pair<std::vector<unsigned char>, std::vector<unsigned char>>* Context::getCertsByHostname(const std::string& hostname) const {
	auto certIter = certsByHostname.find(hostname);
	return certIter == std::end(certsByHostname) ? nullptr : &certIter->second;
}

void Context::addProcedure(std::unique_ptr<esl::processing::procedure::Interface::Procedure> procedure) {
	entries.emplace_back(new EntryImpl(std::move(procedure)));
}

void Context::addProcedure(const std::string& refId) {
	esl::processing::procedure::Interface::Procedure* procedure = findObject<esl::processing::procedure::Interface::Procedure>(refId);

	if(procedure == nullptr) {
	    throw std::runtime_error("No procedure found with ref-id=\"" + refId + "\".");
	}

	entries.emplace_back(new EntryImpl(*procedure));
}

void Context::addBasicServer(std::unique_ptr<basic::Server> server) {
	entries.emplace_back(new EntryImpl(std::move(server)));
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

void Context::procedureRun(esl::object::ObjectContext& objectContext) {
	logger.debug << "Starting all threads ...\n";

	/* ****************************************************************************************************** *
	 * Install stop signal handler.                                                                           *
	 * This should be done before initializing object to be able to terminate initialization (std::terminate) *
	 * ****************************************************************************************************** */
	std::function<void()> signalFunction = [this]() {
		// logger.trace << "SIGNAL FUNCTION: notify signal thread to stop signal thread\n";
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

	try {
		ProcessLockGuard processLockGuard(*this);

		for(auto signalType : stopSignals) {
			esl::system::SignalHandler::install(signalType, signalFunction);
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
			    objectContext.addObject("return-code", std::unique_ptr<esl::object::Interface::Object>(new ReturnCodeObject(exceptionReturnCode)));
			}
		}

		if(catchException) {
			ExceptionHandler exceptionHandler(std::current_exception());
	    	exceptionHandler.dump(std::cerr);

			flushLogAppender("eslx/ostream", *appenderCoutStream);
			flushLogAppender("eslx/membuffer", *appenderMemBuffer);
		}
		else {
			throw;
		}
	}

	/* ************************************************************************** *
	 * Remove stop signal handler and wait for signal thread has been terminated. *
	 * ************************************************************************** */
	for(auto signalType : stopSignals) {
		esl::system::SignalHandler::remove(signalType, signalFunction);
	}
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

	std::set<esl::processing::procedure::Interface::Procedure*> proceduresRunningCopy;
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
				basic::Server* basicServer = dynamic_cast<basic::Server*>(procedure);
				http::Server* httpServer = dynamic_cast<http::Server*>(procedure);

				if(basicServer) {
					logger.debug << "[" << ++proceduresCurrent << "/" << proceduresTotal << "] Stopping basic server initiated\n";
				}
				else if(httpServer) {
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

void Context::processRegister(esl::processing::procedure::Interface::Procedure& procedureRunning) {
	std::lock_guard<std::mutex> proceduresRunningLock(proceduresRunningMutex);
	proceduresRunning.insert(&procedureRunning);
}

void Context::processUnregister(esl::processing::procedure::Interface::Procedure& procedureRunning) {
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

void Context::initializeContext() {
	if(verbose) {
		/* show configuration file */
		dumpTree(0);
		std::cout << "\n\n";
	}

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

void Context::flushLogAppender(const std::string& id, esl::logging::appender::Interface::Appender& appender) {
	std::stringstream strStream;

	appender.flush();
	appender.flush(strStream);

	if(!strStream.str().empty()) {
		std::cerr << "\n\nFlush log messages " << id << ":\n";
		std::cerr << strStream.str();
	}
}

} /* namespace main */
} /* namespace engine */
} /* namespace jerry */
