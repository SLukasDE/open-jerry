/*
 * This file is part of Jerry application server.
 * Copyright (c) 2019-2021 Sven Lukas
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

//#include <jerry/Module.h>
#include <jerry/Logger.h>
//#include <jerry/config/Config.h>
#include <jerry/config/Engine.h>
#include <jerry/engine/Engine.h>
#include <jerry/Module.h>
#include <jerry/engine/ExceptionHandler.h>

#include <esl/logging/Logger.h>
#include <esl/logging/appender/OStream.h>
#include <esl/logging/appender/MemBuffer.h>
#include <esl/logging/Location.h>
#include <esl/logging/Layout.h>
#include <esl/database/exception/SqlError.h>
#include <esl/Module.h>

#include <iostream>
#include <memory>

esl::logging::appender::OStream appenderCoutStream(std::cout, std::cout, std::cout, std::cerr, std::cerr);
esl::logging::appender::OStream appenderCerrStream(std::cerr, std::cerr, std::cerr, std::cerr, std::cerr);
esl::logging::appender::MemBuffer appenderMemBuffer(100);
std::unique_ptr<esl::logging::Layout> layout;


void printInterface(std::ostream& oStream, const std::string& text, const esl::module::Interface* interface) {
	if(interface) {
		oStream << text << ":\n";
		oStream << "  module:         \"" << interface->module << "\"\n";
		oStream << "  type:           \"" << interface->type << "\"\n";
		oStream << "  implementation: \"" << interface->implementation << "\"\n";
		oStream << "  apiVersion:     \"" << interface->apiVersion << "\"\n";
		oStream << "\n";
	}
	else {
		oStream << text << ": NOT FOUND!\n";
	}
}

void printModules(std::ostream& oStream) {
	oStream << "Interfaces:\n";
	oStream << "-----------\n";
	for(const auto& interface : esl::getModule().getMetaInterfaces()) {
		printInterface(oStream, "esl", &interface);
	}
	/*
	 for(const auto& interface : jerry::getModule().getMetaInterfaces()) {
		printInterface(oStream, "jerry", &interface);
	}
	*/
	oStream << "\n";
	oStream << "\n";
}

void printUsage() {
	std::cout << "Wrong arguments.\n";
	std::cout << "Usage: jerry [-cgi] [-v] <path to server.xml>\n";
	std::cout << "  -cgi\n";
	std::cout << "    Specifying this flags switches the server to a CGI client that processes just one request with output on STDOUT.\n";
	std::cout << "  -v\n";
	std::cout << "    Specifying this flags results to some extra output on startup phase.\n";
	std::cout << "  -d\n";
	std::cout << "    Run this server as daemon.\n";
	std::cout << "  -dry\n";
	std::cout << "    Make a dry run. Just read server configuration file and load libraries.\n";
	std::cout << "  <path to server.xml>\n";
	std::cout << "    This file is mandatory. It contains the whole server configuration.\n";
}

int findFlagIndex(int argc, const char *argv[], const std::string& flag) {
	for(int i=1; i<argc; ++i) {
		if(std::string(argv[i]) == flag) {
			return i;
		}
	}
	return -1;
}

int calcFlagIndexSum(int argc, int flags) {
	int rc = flags * (-1);

	for(int i=1; i<argc; ++i) {
		rc += 1 + i;
	}

	return rc;
}

int main(int argc, const char *argv[]) {
	try {
		if(argc > 5) {
			printUsage();
			return -1;
		}

		int flagIndexCGI = findFlagIndex(argc, argv, "-cgi");
		bool isCGI = (flagIndexCGI > 0);

		int flagIndexVerbose = findFlagIndex(argc, argv, "-v");
		bool isVerbose = (flagIndexVerbose > 0);

		int flagIndexDaemon = findFlagIndex(argc, argv, "-d");
		bool isDaemon = (flagIndexDaemon > 0);

		int flagIndexDryRun = findFlagIndex(argc, argv, "-dry");
		bool isDryRun = (flagIndexDryRun > 0);

		int flagIndexConfigFile = -1;
		for(int i=1; i<argc; ++i) {
			if(flagIndexCGI != i
			&& flagIndexVerbose != i
			&& flagIndexDaemon != i
			&& flagIndexDryRun != i) {
				flagIndexConfigFile = i;
				break;
			}
		}
		std::string configFile;
		if(flagIndexConfigFile > 0) {
			configFile = argv[flagIndexConfigFile];
		}
		else {
			printUsage();
			return -1;
		}

		int flagIndexSum1 = flagIndexCGI + flagIndexVerbose + flagIndexDryRun + flagIndexConfigFile + flagIndexDaemon;
		int flagIndexSum2 = calcFlagIndexSum(argc, 5);
		if(flagIndexSum1 != flagIndexSum2) {
			printUsage();
			return -1;
		}

		jerry::Module::install(esl::getModule());

		jerry::config::Engine xmlEngine(configFile);
		jerry::engine::Engine engine;
#if 1
		if(isCGI) {
			layout = xmlEngine.install(engine, appenderCerrStream, appenderMemBuffer);
		}
		else {
			layout = xmlEngine.install(engine, appenderCoutStream, appenderMemBuffer);
		}
#else
		/* load libraries and show registered modules */
		xmlEngine.loadLibraries();

		layout = xmlEngine.createLayout();

		if(isCGI) {
		    appenderCerrStream.setRecordLevel();
		    appenderCerrStream.setLayout(layout.get());
		    esl::logging::addAppender(appenderCerrStream);
		}
		else {
		    appenderCoutStream.setRecordLevel();
		    appenderCoutStream.setLayout(layout.get());
		    esl::logging::addAppender(appenderCoutStream);
		}


		/* MemBuffer appender just writes output to a buffer of a fixed number of lines.
		 * If number of columns is specified as well the whole memory is allocated at initialization time.
		 */
		appenderMemBuffer.setRecordLevel(esl::logging::Appender::RecordLevel::ALL);
		appenderMemBuffer.setLayout(layout.get());
		esl::logging::addAppender(appenderMemBuffer);

		xmlEngine.setLogLevel();
		xmlEngine.install(engine);
		//config.setEngine(engine);
#endif
		if(isVerbose) {
			std::ostream& oStream = isCGI ? std::cerr : std::cout;

			/* show configuration file */
			xmlEngine.save(oStream);

			oStream << "\n\n";

			/* show loaded modules and interfaces */
			printModules(oStream);

			/* show configuration file */
			engine.dumpTree(0);
		}

		if(!isDryRun) {
			if(isCGI) {
				if(!engine.runCGI()) {
					return -1;
				}
			}
			else {
				if(!engine.run(isDaemon)) {
					return -1;
				}
			}
		}
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

    	return -1;
	}

	return 0;
}
