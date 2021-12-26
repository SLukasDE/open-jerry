/*
 * This file is part of Jerry application server.
 * Copyright (C) 2020-2021 Sven Lukas
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
#include <jerry/engine/basic/server/Socket.h>
#include <jerry/engine/http/server/Socket.h>
#include <jerry/Logger.h>

#include <esl/Module.h>
#include <esl/com/basic/client/Interface.h>
#include <esl/com/http/client/Interface.h>
#include <esl/processing/daemon/Interface.h>
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


Engine::Engine()
: ObjectContext(true)
{ }

void Engine::addCertificate(const std::string& hostname, std::vector<unsigned char> key, std::vector<unsigned char> certificate) {
	std::pair<std::vector<unsigned char>, std::vector<unsigned char>>& certPair = certsByHostname[hostname];
	certPair.first = std::move(certificate);
	certPair.second = std::move(key);
}

void Engine::addCertificate(const std::string& hostname, const std::string& keyFile, const std::string& certificateFile) {
    std::ifstream keyIFStream(keyFile, std::ios::binary );
    std::ifstream certificateIFStream(certificateFile, std::ios::binary );

    // copies all data into buffer
    auto key = std::vector<unsigned char>(std::istreambuf_iterator<char>(keyIFStream), {});
    auto certificate = std::vector<unsigned char>(std::istreambuf_iterator<char>(certificateIFStream), {});

//    logger.info << "Key size: " << key.size() << "\n";
//    logger.info << "Certificate size: " << certificate.size() << "\n";

    addCertificate(hostname, std::move(key), std::move(certificate));
}

const std::map<std::string, std::pair<std::vector<unsigned char>, std::vector<unsigned char>>>& Engine::getCertificates() const noexcept {
	return certsByHostname;
}

const std::pair<std::vector<unsigned char>, std::vector<unsigned char>>* Engine::getCertsByHostname(const std::string& hostname) const {
	auto certIter = certsByHostname.find(hostname);
	return certIter == std::end(certsByHostname) ? nullptr : &certIter->second;
}

basic::server::Context& Engine::addBasicServer(const std::vector<std::pair<std::string, std::string>>& settings, const std::string& implementation) {
	logger.trace << "Adding basic server (implementation=\"" << implementation << "\")\n";

	std::unique_ptr<basic::server::Socket> socket(new basic::server::Socket(settings, implementation));
	basic::server::Context& context = socket->getContext();
	basicServers.push_back(std::move(socket));

	bool inheritObjects = true;
	if(inheritObjects) {
		context.ObjectContext::setParent(this);
	}

	return context;
}

http::server::Context& Engine::addHttpServer(bool isHttps, const std::vector<std::pair<std::string, std::string>>& settings, const std::string& implementation) {
	if(isHttps) {
		logger.trace << "Adding HTTPS server (implementation=\"" << implementation << "\")\n";
	}
	else {
		logger.trace << "Adding HTTP server (implementation=\"" << implementation << "\")\n";
	}

	std::unique_ptr<http::server::Socket> socket(new http::server::Socket(isHttps, settings, implementation));
	http::server::Context& context = socket->getContext();
	httpServers.push_back(std::move(socket));

	bool inheritObjects = true;
	if(inheritObjects) {
		context.ObjectContext::setParent(this);
	}

	return context;
}

void Engine::addDaemon(const std::vector<std::pair<std::string, std::string>>& settings, const std::string& implementation) {
	logger.trace << "Adding daemon (implementation=\"" << implementation << "\")\n";
	std::unique_ptr<esl::processing::daemon::Interface::Daemon> daemon = esl::getModule().getInterface<esl::processing::daemon::Interface>(implementation).createDaemon(settings);
	if(!daemon) {
		throw std::runtime_error("Cannot create an daemon for implementation '" + implementation + "' because interface method createDaemon() returns nullptr.");
	}

	daemons.push_back(std::move(daemon));
}

void Engine::addBasicClient(const std::string& id, const std::vector<std::pair<std::string, std::string>>& settings, const std::string& implementation) {
	logger.trace << "Adding basic client (implementation=\"" << implementation << "\") with id=\"" << id << "\"\n";
	std::unique_ptr<esl::com::basic::client::Interface::ConnectionFactory> connectionFactory = esl::getModule().getInterface<esl::com::basic::client::Interface>(implementation).createConnectionFactory(settings);
	if(!connectionFactory) {
		throw std::runtime_error("Cannot create an basic connection-factory with id '" + id + "' for implementation '" + implementation + "' because interface method createConnectionFactory() returns nullptr.");
	}

	addObject(id, std::unique_ptr<esl::object::Interface::Object>(connectionFactory.release()));
}

void Engine::addHttpClient(const std::string& id, const std::string& url, const std::vector<std::pair<std::string, std::string>>& settings, const std::string& implementation) {
	logger.trace << "Adding http client (implementation=\"" << implementation << "\") with id=\"" << id << "\" and url=\"" + url + "\"\n";
	std::unique_ptr<esl::com::http::client::Interface::ConnectionFactory> connectionFactory = esl::getModule().getInterface<esl::com::http::client::Interface>(implementation).createConnectionFactory(url, settings);
	if(!connectionFactory) {
		throw std::runtime_error("Cannot create an basic connection-factory with id '" + id + "' for implementation '" + implementation + "' because interface method createConnectionFactory() returns nullptr.");
	}

	addObject(id, std::unique_ptr<esl::object::Interface::Object>(connectionFactory.release()));
}

const std::vector<std::unique_ptr<basic::server::Socket>>& Engine::getBasicServers() const {
	return basicServers;
}

const std::vector<std::unique_ptr<http::server::Socket>>& Engine::getHttpServers() const {
	return httpServers;
}

const std::vector<std::unique_ptr<esl::processing::daemon::Interface::Daemon>>& Engine::getDaemons() const {
	return daemons;
}

void Engine::initializeContext() {
	ObjectContext::initializeContext();

	for(const auto& socket: basicServers) {
		socket->initializeContext();
	}

	for(const auto& socket: httpServers) {
		socket->initializeContext();
	}

	for(const auto& daemon: daemons) {
		esl::object::InitializeContext* initializeContext = dynamic_cast<esl::object::InitializeContext*>(daemon.get());
		if(initializeContext) {
			initializeContext->initializeContext(*this);
		}
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
