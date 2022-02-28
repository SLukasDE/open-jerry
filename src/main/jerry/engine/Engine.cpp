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

#include <jerry/engine/Engine.h>
#include <jerry/engine/ObjectContext.h>
#include <jerry/engine/basic/Socket.h>
#include <jerry/engine/http/Socket.h>
#include <jerry/Logger.h>

#include <esl/Module.h>
#include <esl/object/InitializeContext.h>
#include <esl/Stacktrace.h>
#include <esl/utility/String.h>

#include <set>
#include <fstream>
#include <vector>
#include <algorithm>
#include <stdexcept>


namespace jerry {
namespace engine {

namespace {
Logger logger("jerry::engine::Engine");
} /* anonymous namespace */


Engine::Engine(std::function<void()> aOnProcessRegister, std::function<void()> aOnProcessUnregister)
: ObjectContext(true),
  onProcessRegister(aOnProcessRegister),
  onProcessUnregister(aOnProcessUnregister)
{ }

void Engine::startServers() {
	const std::size_t processesTotal = basicServers.size() + httpServers.size() + daemons.size();
	logger.debug << "Starting " << processesTotal << " server processes ...\n";

	std::size_t processesCurrent = 0;

	for(const auto& socket: basicServers) {
		if(logger.debug) {
			std::set<std::string> notifiers = socket->getNotifiers();
			for(const auto& notifier : notifiers) {
				logger.debug << "   - \"" << notifier << "\"\n";
			}
		}
		socket->listen(onProcessUnregister);
		onProcessRegister();
		logger.debug << "[" << ++processesCurrent << "/" << processesTotal << "] Basic server started\n";
	}

	for(const auto& socket: httpServers) {
		socket->listen(onProcessUnregister);
		onProcessRegister();
		logger.debug << "[" << ++processesCurrent << "/" << processesTotal << "] HTTP/HTTPS server started\n";
	}

	for(const auto& daemon: daemons) {
		daemon->start(onProcessUnregister);
		onProcessRegister();
		logger.debug << "[" << ++processesCurrent << "/" << processesTotal << "] Daemon started\n";
	}

	logger.debug << "All server processes started.\n";
}

void Engine::stopServers() {
	const std::size_t processesTotal = basicServers.size() + httpServers.size() + daemons.size();
	logger.debug << "Stopping " << processesTotal << " server processes ...\n";

	std::size_t processesCurrent = 0;

	for(const auto& socket: basicServers) {
		socket->release();
		logger.debug << "[" << ++processesCurrent << "/" << processesTotal << "] Stopping basic server initiated\n";
	}

	for(const auto& socket: httpServers) {
		socket->release();
		logger.debug << "[" << ++processesCurrent << "/" << processesTotal << "] Stopping HTTP/HTTPS server initiated\n";
	}

	for(const auto& daemon: daemons) {
		daemon->release();
		logger.debug << "[" << ++processesCurrent << "/" << processesTotal << "] Stopping daemon initiated\n";
	}

	logger.debug << "Stopping initiated for all server processes.\n";
}

void Engine::addCertificate(const std::string& hostname, std::vector<unsigned char> key, std::vector<unsigned char> certificate) {
	std::pair<std::vector<unsigned char>, std::vector<unsigned char>>& certPair = certsByHostname[hostname];
	certPair.first = std::move(certificate);
	certPair.second = std::move(key);
}

void Engine::addCertificate(const std::string& hostname, const std::string& keyFile, const std::string& certificateFile) {
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

const std::map<std::string, std::pair<std::vector<unsigned char>, std::vector<unsigned char>>>& Engine::getCertificates() const noexcept {
	return certsByHostname;
}

const std::pair<std::vector<unsigned char>, std::vector<unsigned char>>* Engine::getCertsByHostname(const std::string& hostname) const {
	auto certIter = certsByHostname.find(hostname);
	return certIter == std::end(certsByHostname) ? nullptr : &certIter->second;
}

void Engine::addObject(const std::string& id, std::unique_ptr<esl::object::Interface::Object> object) {
	if(id.empty()) {
		throw std::runtime_error("Multiple anonymous object is not allowed.");
	}
	ObjectContext::addObject(id, std::move(object));
}

void Engine::addBatchProcedure(const std::vector<std::pair<std::string, std::string>>& settings, const std::string& implementation) {
	logger.trace << "Adding batch procedure (implementation=\"" << implementation << "\")\n";

	if(batchProcedure) {
		throw std::runtime_error("Multiple definitions of batch procedures are not allowed.");
	}
	batchProcedure = esl::getModule().getInterface<esl::processing::procedure::Interface>(implementation).createProcedure(settings);
	if(!batchProcedure) {
		throw std::runtime_error("Cannot create a batch procedure for implementation '" + implementation + "' because interface method createProcedure() returns nullptr.");
	}
	batchProcedurePtr = batchProcedure.get();
}

void Engine::setBatchProcedure(esl::processing::procedure::Interface::Procedure* aBatchProcedure) {
	batchProcedurePtr = aBatchProcedure;
}

esl::processing::procedure::Interface::Procedure* Engine::getBatchProcedure() const noexcept {
	return batchProcedurePtr;
}

void Engine::addDaemonProcedure(const std::vector<std::pair<std::string, std::string>>& settings, const std::string& implementation) {
	logger.trace << "Adding daemon procedure (implementation=\"" << implementation << "\")\n";

	std::unique_ptr<esl::processing::procedure::Interface::Procedure> procedure = esl::getModule().getInterface<esl::processing::procedure::Interface>(implementation).createProcedure(settings);
	if(!procedure) {
		throw std::runtime_error("Cannot create a Procedure for implementation '" + implementation + "' because interface method createProcedure() returns nullptr.");
	}

	std::unique_ptr<Daemon> daemon(new Daemon(std::move(procedure)));
	daemons.push_back(std::move(daemon));
}

basic::Socket& Engine::addBasicServer(const std::vector<std::pair<std::string, std::string>>& settings, const std::string& implementation) {
	logger.trace << "Adding basic server (implementation=\"" << implementation << "\")\n";

	std::unique_ptr<basic::Socket> socketPtr(new basic::Socket(settings, implementation));
	basic::Socket& socket = *socketPtr;
	basicServers.push_back(std::move(socketPtr));
	return socket;
}

http::Socket& Engine::addHttpServer(bool isHttps, const std::vector<std::pair<std::string, std::string>>& settings, const std::string& implementation) {
	if(isHttps) {
		logger.trace << "Adding HTTPS server (implementation=\"" << implementation << "\")\n";
	}
	else {
		logger.trace << "Adding HTTP server (implementation=\"" << implementation << "\")\n";
	}

	std::unique_ptr<http::Socket> socketPtr(new http::Socket(isHttps, settings, implementation));
	http::Socket& socket = *socketPtr;
	httpServers.push_back(std::move(socketPtr));
	return socket;
}

void Engine::initializeContext() {
	/* *********************************************************** *
	 * add certificates to socket if http-server is used for https *
	 * *********************************************************** */
	for(const auto& httpSocket: httpServers) {
		if(httpSocket->isHttps()) {
			if(getCertificates().empty()) {
				throw std::runtime_error("No certificates are available.");
			}
			for(const auto& certificate : getCertificates()) {
				httpSocket->addTLSHost(certificate.first, certificate.second.first, certificate.second.second);
			}
		}
	}


	ObjectContext::initializeContext();

	if(batchProcedurePtr) {
		esl::object::InitializeContext* initializeContext = dynamic_cast<esl::object::InitializeContext*>(batchProcedurePtr);
		if(initializeContext) {
			initializeContext->initializeContext(*this);
		}
	}

	for(const auto& socket: basicServers) {
		socket->initializeContext();
	}

	for(const auto& socket: httpServers) {
		socket->initializeContext();
	}

	for(const auto& daemon: daemons) {
		daemon->initializeContext(*this);
	}
}

void Engine::dumpTree(std::size_t depth) const {
	for(std::size_t i=0; i<depth; ++i) {
		logger.info << "|   ";
	}
	logger.info << "+-> Engine\n";
	++depth;

	ObjectContext::dumpTree(depth);
	dumpTreeBasicServers(depth);
	dumpTreeHttpServers(depth);
	dumpTreeDaemons(depth);
}

void Engine::dumpTreeBasicServers(std::size_t depth) const {
	for(const auto& socket: basicServers) {
		for(std::size_t i=0; i<depth; ++i) {
			logger.info << "|   ";
		}
		logger.info << "+-> Basic server: -> " << socket.get() << "\n";
		socket->dumpTree(depth + 1);
	}
}

void Engine::dumpTreeHttpServers(std::size_t depth) const {
	for(const auto& socket: httpServers) {
		for(std::size_t i=0; i<depth; ++i) {
			logger.info << "|   ";
		}
		logger.info << "+-> HTTP server: -> " << socket.get() << "\n";
		socket->dumpTree(depth + 1);
	}
}

void Engine::dumpTreeDaemons(std::size_t depth) const {
	for(const auto& daemon: daemons) {
		for(std::size_t i=0; i<depth; ++i) {
			logger.info << "|   ";
		}
		logger.info << "+-> Daemon: -> " << daemon.get() << "\n";
	}
}

} /* namespace engine */
} /* namespace jerry */
