#include <esl/Module.h>
#include <esl/module/Library.h>
#include <esl/Stacktrace.h>

#include <jerry/Module.h>
#include <jerry/Logger.h>
#include <jerry/URL.h>
#include <jerry/config/Jerry.h>
#include <jerry/engine/Engine.h>
#include <jerry/engine/Listener.h>

#include <esl/object/parameter/Interface.h>

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
	try {
		try {
			return runServer("/home/lukas/workspace_esl/jerry/server2.xml");
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
