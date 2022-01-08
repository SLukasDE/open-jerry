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

#include <jerry/Daemon.h>
#include <jerry/Logger.h>
#include <jerry/Module.h>
#include <jerry/engine/ExceptionHandler.h>
#include <jerry/config/Engine.h>

#include <esl/system/SignalHandler.h>
#include <esl/logging/Logger.h>
#include <esl/logging/appender/OStream.h>
#include <esl/logging/appender/MemBuffer.h>
#include <esl/logging/Layout.h>
#include <esl/Module.h>
//#include <esl/module/Library.h>

#include <iostream>

namespace jerry {

namespace {
esl::logging::appender::OStream appenderCoutStream(std::cout, std::cout, std::cout, std::cerr, std::cerr);
esl::logging::appender::OStream appenderCerrStream(std::cerr, std::cerr, std::cerr, std::cerr, std::cerr);
esl::logging::appender::MemBuffer appenderMemBuffer(100);
std::unique_ptr<esl::logging::Layout> layout;

void printInterface(const std::string& text, const esl::module::Interface* interface) {
	if(interface) {
		std::cout << text << ":\n";
		std::cout << "  module:         \"" << interface->module << "\"\n";
		std::cout << "  type:           \"" << interface->type << "\"\n";
		std::cout << "  implementation: \"" << interface->implementation << "\"\n";
		std::cout << "  apiVersion:     \"" << interface->apiVersion << "\"\n";
		std::cout << "\n";
	}
	else {
		std::cout << text << ": NOT FOUND!\n";
	}
}

void printModules() {
	std::cout << "Interfaces:\n";
	std::cout << "-----------\n";
	for(const auto& interface : esl::getModule().getMetaInterfaces()) {
		printInterface("esl", &interface);
	}
	/*
	 for(const auto& interface : jerry::getModule().getMetaInterfaces()) {
		printInterface(oStream, "jerry", &interface);
	}
	*/
	std::cout << "\n";
	std::cout << "\n";
}

Logger logger("jerry::Daemon");
} /* anonymous namespace */

Daemon::Daemon()
: messageTimer([this](const State& messageState) {
	if(messageState == started) {
		start(nullptr);
	}
	else if(messageState == stopping) {
		release();
	}
	else if(messageState == stopped) {
		messageTimer.stop();
	}
})
{ }

Daemon::~Daemon() {
	wait(0);
}

bool Daemon::setupXML(const std::string& configFile, bool verbose) {
	try {
		Module::install(esl::getModule());

		jerry::config::Engine xmlEngine(configFile);
		layout = xmlEngine.install(getEngine(), appenderCoutStream, appenderMemBuffer);

		if(verbose) {
			/* show configuration file */
			xmlEngine.save(std::cout);

			std::cout << "\n\n";

			/* show loaded modules and interfaces */
			printModules();

			/* show configuration file */
			getEngine().dumpTree(0);
		}
	}
	catch(...) {
		jerry::engine::ExceptionHandler exceptionHandler(std::current_exception());
    	exceptionHandler.dump(std::cerr);

		return false;
	}

	return true;
}

bool Daemon::run() {
	bool success = true;

	auto stopFunction = [this]() {
		stopSignal();
	};

	esl::system::SignalHandler::install(esl::system::SignalHandler::SignalType::interrupt, stopFunction);
	esl::system::SignalHandler::install(esl::system::SignalHandler::SignalType::terminate, stopFunction);
	esl::system::SignalHandler::install(esl::system::SignalHandler::SignalType::pipe, stopFunction);

	try {
		messageTimer.addMessage(0, started, std::chrono::milliseconds(0), false);
		messageTimer.run();
	}
	catch(...) {
		jerry::engine::ExceptionHandler exceptionHandler(std::current_exception());
    	exceptionHandler.dump(std::cerr);

    	std::cerr << "\n\nReplay previous log messages:\n";
        auto buffer = appenderMemBuffer.getBuffer();
        if(layout) {
            for(const auto& entry : buffer) {
            	std::cerr << layout->toString(std::get<0>(entry)) << std::get<1>(entry) << "\n";
            }
        }
        else {
            for(const auto& entry : buffer) {
            	std::cerr << std::get<1>(entry) << "\n";
            }
        }

    	success = false;
	}

	esl::system::SignalHandler::remove(esl::system::SignalHandler::SignalType::pipe, stopFunction);
	esl::system::SignalHandler::remove(esl::system::SignalHandler::SignalType::terminate, stopFunction);
	esl::system::SignalHandler::remove(esl::system::SignalHandler::SignalType::interrupt, stopFunction);

	return success;
}

engine::Engine& Daemon::getEngine() {
	return jEngine;
}

bool Daemon::start(std::function<void()> aOnReleasedHandler) {
	std::lock_guard<std::mutex> stateLock(stateMutex);
	if(state != stopped) {
		messageTimer.stop();
		return false;
	}

	initialize();

	onReleasedHandler = aOnReleasedHandler;

	const std::size_t processesTotal = jEngine.getBasicServers().size() + jEngine.getHttpServers().size() + jEngine.getDaemons().size();
	std::size_t processesCurrent = 0;
	logger.debug << "Starting " << processesTotal << " processes ...\n";

	for(const auto& socket: jEngine.getBasicServers()) {
		if(logger.debug) {
			std::set<std::string> notifiers = socket->getNotifiers();
			for(const auto& notifier : notifiers) {
				logger.debug << "   - \"" << notifier << "\"\n";
			}
		}
		socket->listen([this]() {
			onReleased();
		});
		++runningStates;
		logger.debug << "[" << ++processesCurrent << "/" << processesTotal << "] Basic server started\n";
	}

	for(const auto& socket: jEngine.getHttpServers()) {
		socket->listen([this]() {
			onReleased();
		});
		++runningStates;
		logger.debug << "[" << ++processesCurrent << "/" << processesTotal << "] HTTP/HTTPS server started\n";
	}

	for(const auto& daemon: jEngine.getDaemons()) {
		daemon->start([this]() {
			onReleased();
		});
		++runningStates;
		logger.debug << "[" << ++processesCurrent << "/" << processesTotal << "] Daemon started\n";
	}

	logger.debug << "All processes started.\n";

	if(runningStates > 0) {
		state = started;
	}
	else {
		messageTimer.stop();
	}

	return true;
}

void Daemon::release() {
	{
		std::lock_guard<std::mutex> stateLock(stateMutex);
		if(state != started) {
			return;
		}
		state = stopping;
	}

	const std::size_t processesTotal = jEngine.getBasicServers().size() + jEngine.getHttpServers().size() + jEngine.getDaemons().size();
	std::size_t processesCurrent = 0;
	logger.debug << "Shutdown " << processesTotal << " processes ...\n";

	for(const auto& socket: jEngine.getBasicServers()) {
		socket->release();
		logger.debug << "[" << ++processesCurrent << "/" << processesTotal << "] Basic server shutdown initiated\n";
	}

	for(const auto& socket: jEngine.getHttpServers()) {
		socket->release();
		logger.debug << "[" << ++processesCurrent << "/" << processesTotal << "] HTTP/HTTPS server shutdown initiated\n";
	}

	for(const auto& daemon: jEngine.getDaemons()) {
		daemon->release();
		logger.debug << "[" << ++processesCurrent << "/" << processesTotal << "] Daemon shutdown initiated\n";
	}

	logger.debug << "Shutdown for all processes initiated.\n";
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

void Daemon::onReleased() {
	logger.debug << "something has been stopped\n";
	{
		std::lock_guard<std::mutex> stateLock(stateMutex);
		--runningStates;
		logger.debug << "runningStates: " <<  runningStates << "\n";

		if(runningStates == 0) {
			state = stopped;
			if(onReleasedHandler) {
				onReleasedHandler();
				onReleasedHandler = nullptr;
			}
			messageTimer.stop();
		}
	}
	stateNotifyCondVar.notify_all();
}

void Daemon::stopSignal() {
	if(stopSignalCounter == 0) {
		std::terminate();
	}
	if(stopSignalCounter > 0) {
		--stopSignalCounter;
	}
	logger.info << "stopping engine\n";
	messageTimer.addMessage(0, stopping, std::chrono::milliseconds(0), false);
}

void Daemon::initialize() {
	/* *********************************************************** *
	 * initialize ExceptionHandler:                                *
	 * Load all implementations to convert 'const std::exception&' *
	 * to esl::http::exception::Interface::Message         *
	 * *********************************************************** */
	if(isInitialized) {
		return;
	}

	/* *********************************************************** *
	 * add certificates to socket if http-server is used for https *
	 * *********************************************************** */
	for(const auto& httpServer: jEngine.getHttpServers()) {
		if(httpServer->isHttps()) {
			if(jEngine.getCertificates().empty()) {
				throw std::runtime_error("No certificates are available.");
			}
			for(const auto& certificate : jEngine.getCertificates()) {
				httpServer->addTLSHost(certificate.first, certificate.second.first, certificate.second.second);
			}
		}
	}

	/* ************************* *
	 * initialize global objects *
	 * ************************* */
	logger.info << "Initialize objects ...\n";
	jEngine.initializeContext();
	logger.info << "Initialization done.\n";

	isInitialized = true;
}

} /* namespace jerry */
