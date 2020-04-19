/*
 * This file is part of Jerry application server.
 * Copyright (C) 2020 Sven Lukas
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

#include <esl/Module.h>
#include <esl/module/Library.h>
#include <esl/Stacktrace.h>

#include <jerry/Module.h>
#include <jerry/Logger.h>
//#include <jerry/utility/URL.h>
#include <jerry/config/Jerry.h>
#include <jerry/engine/Engine.h>
#include <jerry/engine/Listener.h>

//#include <esl/object/parameter/Interface.h>

#include <esl/logging/Logger.h>
#include <esl/logging/appender/OStream.h>
#include <esl/logging/appender/MemBuffer.h>
#include <esl/logging/Location.h>
#include <esl/logging/Layout.h>

#include <iostream>
#include <memory>

bool isVerbose = false;
esl::logging::appender::OStream appenderOstream(std::cout, std::cout, std::cout, std::cerr, std::cerr);
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
	for(const auto& interface : esl::getModule().getInterfaces()) {
		printInterface("esl", &interface);
	}
	for(const auto& interface : jerry::getModule().getInterfaces()) {
		printInterface("jerry", &interface);
	}
	std::cout << "\n";
	std::cout << "\n";
}

int runServer(const std::string& configFile) {
	jerry::Logger logger("jerry::runServer");

	/* load and show config file */
	jerry::config::Jerry config(configFile);
	if(isVerbose) {
		config.print();
	}

	/* load libraries and show registered modules */
	config.loadLibraries();
	if(isVerbose) {
		printModules();
	}

	layout = config.getLayout();

    appenderOstream.setRecordLevel();
	appenderOstream.setLayout(layout.get());
    esl::logging::addAppender(appenderOstream);


	/* MemBuffer appender just writes output to a buffer of a fixed number of lines.
	 * If number of columns is specified as well the whole memory is allocated at initialization time.
	 */
	appenderMemBuffer.setRecordLevel(esl::logging::Appender::RecordLevel::ALL);
	appenderMemBuffer.setLayout(layout.get());
	esl::logging::addAppender(appenderMemBuffer);

	jerry::engine::Engine engine;

	config.setEngine(engine);

	return engine.run();
}

int main(int argc, const char *argv[]) {
	if(argc != 2) {
		std::cout << "Wrong number of arguments.\n";
		std::cout << "Usage: jerry <path to server.xml>\n";
		return 0;
	}

	try {
		try {
			return runServer(argv[1]);
		}
		catch(const std::runtime_error& e) {
			std::cerr << "Exception : std::runtime_error\n";
			std::cerr << "Message   : " << e.what() << "\n";
			throw;
		}
		catch(const std::exception& e) {
			std::cerr << "Exception : std::exception\n";
			std::cerr << "Message   : " << e.what() << "\n";
			throw;
		}
		catch(...) {
			std::cerr << "Exception: unknown\n";
		}
	}
	catch(const std::exception& e) {
		const esl::Stacktrace* stacktrace = esl::getStacktrace(e);
		if(stacktrace) {
			std::cerr << "Stacktrace:\n";
			stacktrace->dump(std::cerr);
		}
		else {
			std::cerr << "Stacktrace: not available\n";
		}
	}
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

	return -1;
}
