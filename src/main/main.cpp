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
#include <jerry/ObjectContext.h>
#include <jerry/ExceptionHandler.h>
#include <jerry/Module.h>

#include <esl/object/Value.h>

#include <string>
#include <vector>
#include <utility>
#include <iostream>

extern const std::string jerryVersionStr;

using ReturnCodeObject = esl::object::Value<int>;

#define _STRINGIFY(x) #x
#define STRINGIFY(x) _STRINGIFY(x)
const std::string jerryVersionStr = STRINGIFY(TRANSFORMER_ARTEFACT_VERSION);

void printUsage() {
	std::cout << "Wrong arguments for jerry " + jerryVersionStr + ".\n";
	std::cout << "Usage: jerry [-v] <path to server.xml>\n";
	std::cout << "  -v\n";
	std::cout << "    Specifying this flags results to some extra output on startup phase.\n";
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
	jerry::Module::install(esl::getModule());

	if(argc > 5) {
		printUsage();
		return -1;
	}

	int flagIndexVerbose = findFlagIndex(argc, argv, "-v");
	bool isVerbose = (flagIndexVerbose > 0);

	int flagIndexDryRun = findFlagIndex(argc, argv, "-dry");
	bool isDryRun = (flagIndexDryRun > 0);

	int flagIndexConfigFile = -1;
	for(int i=1; i<argc; ++i) {
		if(flagIndexVerbose != i
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

	int flagIndexSum1 = flagIndexVerbose + flagIndexDryRun + flagIndexConfigFile;
	int flagIndexSum2 = calcFlagIndexSum(argc, 3);
	if(flagIndexSum1 != flagIndexSum2) {
		printUsage();
		return -1;
	}

	int returnCode = 0;
	try {
		std::vector<std::pair<std::string, std::string>> settings;
		settings.push_back(std::make_pair("config-file", configFile));
		settings.push_back(std::make_pair("stop-signal", "interrupt"));
		settings.push_back(std::make_pair("stop-signal", "terminate"));
		settings.push_back(std::make_pair("stop-signal", "pipe"));
		if(isVerbose) {
			settings.push_back(std::make_pair("verbose", "true"));
		}

		jerry::engine::main::Context procedure(settings);
		jerry::ObjectContext objectContext;

		if(!isDryRun) {
			procedure.procedureRun(objectContext);
		}

		ReturnCodeObject* returnCodeObject = objectContext.findObject<ReturnCodeObject>("return-code");
		if(returnCodeObject) {
			returnCode = returnCodeObject->get();
		}
	}
	catch(...) {
		jerry::ExceptionHandler exceptionHandler(std::current_exception());
    	exceptionHandler.dump(std::cerr);
    	returnCode = -1;
	}

	return returnCode;
}
