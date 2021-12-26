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

#include <jerry/Daemon.h>

#include <iostream>

void printUsage() {
	std::cout << "Wrong arguments.\n";
	std::cout << "Usage: jerry [-v] <path to server.xml>\n";
//	std::cout << "Usage: jerry [-cgi] [-v] <path to server.xml>\n";
//	std::cout << "  -cgi\n";
//	std::cout << "    Specifying this flags switches the server to a CGI client that processes just one request with output on STDOUT.\n";
	std::cout << "  -v\n";
	std::cout << "    Specifying this flags results to some extra output on startup phase.\n";
//	std::cout << "  -d\n";
//	std::cout << "    Run this server as daemon.\n";
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
	if(argc > 5) {
		printUsage();
		return -1;
	}

	int flagIndexCGI = findFlagIndex(argc, argv, "-cgi");
//	bool isCGI = (flagIndexCGI > 0);

	int flagIndexVerbose = findFlagIndex(argc, argv, "-v");
	bool isVerbose = (flagIndexVerbose > 0);
	int flagIndexDaemon = findFlagIndex(argc, argv, "-d");
//		bool isDaemon = (flagIndexDaemon > 0);

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

	jerry::Daemon daemon;
	bool success = daemon.setupXML(configFile, isVerbose);

	if(success && !isDryRun) {
		success = daemon.run();
	}

	return success ? 0 : -1;
}
