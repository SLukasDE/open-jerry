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

#include <openjerry/config/main/Context.h>
#include <openjerry/engine/main/Context.h>
#include <openjerry/ObjectContext.h>
#include <openjerry/ExceptionHandler.h>

#include <esl/crypto/GTXKeyStore.h>
#include <esl/monitoring/Logging.h>
#include <esl/monitoring/LogbookLogging.h>
#include <esl/object/Value.h>
#include <esl/plugin/Registry.h>
#include <esl/system/Stacktrace.h>
#include <esl/system/DefaultSignalManager.h>
#include <esl/system/DefaultStacktraceFactory.h>

#include <openesl/Plugin.h>

#include <boost/filesystem/path.hpp>

#include <string>
#include <vector>
#include <utility>
#include <iostream>

extern const std::string openjerryVersionStr;

using ReturnCodeObject = esl::object::Value<int>;

#define _STRINGIFY(x) #x
#define STRINGIFY(x) _STRINGIFY(x)
const std::string openjerryVersionStr = STRINGIFY(TRANSFORMER_ARTEFACT_VERSION);

void printUsage() {
	std::cout << "\n";
	std::cout << "Usage: openjerry [-v] [-dry] [-l <esl logger configuration file>] <server configuration file>\n";
	std::cout << "  -v\n";
	std::cout << "    Specifying this flags results to some extra output on startup phase.\n";
	std::cout << "  -dry\n";
	std::cout << "    Make a dry run. Just read server configuration file and load libraries.\n";
	std::cout << "  -l <esl logger configuration file>\n";
	std::cout << "    This file is mandatory. It contains the whole server configuration.\n";
	std::cout << "  <server configuration file>\n";
	std::cout << "    This file is mandatory. It contains the whole server configuration.\n";
	std::cout << std::flush;
}

int findFlagIndex(int argc, const char *argv[], const std::string& flag) {
	for(int i=1; i<argc; ++i) {
		if(std::string(argv[i]) == flag) {
			return i;
		}
	}
	return -1;
}

int main(int argc, const char *argv[]) {
	std::cout << "openjerry version " << openjerryVersionStr << std::endl;

	esl::plugin::Registry& registry(esl::plugin::Registry::get());
	openesl::Plugin::install(registry, nullptr);
	registry.setObject(esl::crypto::GTXKeyStore::createNative());
	registry.setObject(esl::system::DefaultStacktraceFactory::createNative());
	{
		esl::system::DefaultSignalManager::Settings aSettings;
		aSettings.isThreaded = true;
		registry.setObject(esl::system::DefaultSignalManager::createNative(aSettings));
	}

	int flagIndexVerbose = findFlagIndex(argc, argv, "-v");
	bool isVerbose = (flagIndexVerbose > 0);

	int flagIndexDryRun = findFlagIndex(argc, argv, "-dry");
	bool isDryRun = (flagIndexDryRun > 0);

	int flagIndexLoggerConfig = findFlagIndex(argc, argv, "-l");
	std::string loggerConfigFile;
	if(flagIndexLoggerConfig > 0) {
		if(flagIndexLoggerConfig+1 >= argc) {
			std::cerr << "Wrong arguments: logger configuration file is missing." << std::endl;
			printUsage();
			return -1;
		}
		loggerConfigFile = argv[flagIndexLoggerConfig+1];
	}

	std::string serverConfigFile;
	for(int i=1; i<argc; ++i) {
		if(isVerbose && flagIndexVerbose == i) {
			continue;
		}

		if(isDryRun && flagIndexDryRun == i) {
			continue;
		}

		if(!loggerConfigFile.empty() && (flagIndexLoggerConfig == i || flagIndexLoggerConfig+1 == i)) {
			continue;
		}

		serverConfigFile = argv[i];
		if(i+1 < argc) {
			std::cerr << "Unknown argument \"" << argv[i+1] << "\"" << std::endl;
			printUsage();
			return -1;
		}
	}

	if(serverConfigFile.empty()) {
		std::cerr << "Wrong arguments: server configuration file is missing." << std::endl;
		printUsage();
		return -1;
	}

	int returnCode = 0;

	try {
		std::vector<std::pair<std::string, std::string>> settings;
		settings.push_back(std::make_pair("stop-signal", "interrupt"));
		settings.push_back(std::make_pair("stop-signal", "terminate"));
		settings.push_back(std::make_pair("stop-signal", "pipe"));
		openjerry::engine::main::Context mainContext(settings);

		boost::filesystem::path serverConfigPath(serverConfigFile);
		openjerry::config::main::Context mainConfig(serverConfigPath);
		if(isVerbose) {
			/* show configuration */
			mainConfig.save(std::cout);
			std::cout << "\n\n";
		}

		mainConfig.loadLibraries();

		if(!loggerConfigFile.empty()) {
			auto logging = esl::monitoring::LogbookLogging::createNative();
			logging->addFile(loggerConfigFile);
			registry.setObject(std::move(logging));
		}

		if(isVerbose) {
			esl::plugin::Registry::get().dump(std::cout);
		}

		mainConfig.install(mainContext);

		openjerry::ObjectContext objectContext;

		if(!isDryRun) {
			mainContext.procedureRun(objectContext);
		}

		ReturnCodeObject* returnCodeObject = objectContext.findObject<ReturnCodeObject>("return-code");
		if(returnCodeObject) {
			returnCode = returnCodeObject->get();
		}
	}
	catch(...) {
		openjerry::ExceptionHandler exceptionHandler(std::current_exception());
    	exceptionHandler.dump(std::cerr);
    	returnCode = -1;
	}

	return returnCode;
}
