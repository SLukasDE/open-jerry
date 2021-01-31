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
#include <jerry/engine/messaging/MessageHandler.h>
#include <jerry/engine/http/RequestHandler.h>
#include <jerry/engine/http/Endpoint.h>
#include <jerry/engine/ExceptionHandler.h>
#include <jerry/Logger.h>
#include <jerry/Module.h>

#include <esl/Module.h>
#include <esl/messaging/Consumer.h>
#include <esl/http/server/Request.h>
#include <esl/http/server/RequestContext.h>
#include <esl/http/server/exception/StatusCode.h>
#include <esl/http/server/requesthandler/Interface.h>
#include <esl/utility/String.h>
#include <esl/system/SignalHandler.h>
#include <esl/Stacktrace.h>

#include <set>
#include <fstream>
#include <vector>
#include <algorithm>
#include <stdexcept>

namespace jerry {
namespace engine {

namespace {
Logger logger("jerry::engine::Engine");

class MessageBrokerObject : public esl::object::Interface::Object {
public:
	MessageBrokerObject(messaging::proxy::Client& aMessageBroker, Engine& aEngine)
	: messageBroker(aMessageBroker),
	  engine(aEngine)
	{ }

	messaging::proxy::Client& getMessageBroker() const {
		return messageBroker;
	}

	Engine& getEngine() const {
		return engine;
	}

private:
	messaging::proxy::Client& messageBroker;
	Engine& engine;
};
} /* anonymous namespace */

Engine::Engine()
: messageTimer([this](const Message& message) { message.f(*this); }),
  httpObject(*this)
{ }

bool Engine::run() {
	runThreadId = std::this_thread::get_id();
	if(httpListenersByPort.empty() && messageListenerByBroker.empty()) {
		logger.warn << "no listener defined.\n";
		return false;
	}

	/* *********************************************************** *
	 * initialize ExceptionHandler:                                *
	 * Load all implementations to convert 'const std::exception&' *
	 * to esl::http::server::exception::Interface::Message         *
	 * *********************************************************** */
	ExceptionHandler::initialize();

	/* ************************* *
	 * initialize global objects *
	 * ************************* */
	logger.info << "Initialize context of global objects...\n";
	initializeContext();
	logger.info << "Initialization of global objects done.\n";

	/* ******************************** *
	 * initialize message-listener objects *
	 * ******************************** */
	for(auto& entry : messageBrokerById) {
		messaging::proxy::Client& messageBroker = entry.second.get();

		logger.info << "Initialize context of all objects within message-listener of broker \"" << messageBroker.id << "\" ...\n";
		for(auto& listener : messageBroker.listeners) {
			listener->initializeContext();
		}
		logger.info << "Initialization of all objects within message-listener of broker \"" << messageBroker.id << "\" done.\n";
	}

	/* ******************************** *
	 * initialize http-listener objects *
	 * ******************************** */
	for(auto& httpListeners : httpListenersByPort) {
		for(auto& listener : httpListeners.second.listenerByHostname) {
			logger.info << "Initialize context of objects for http-listener \"" << listener.first << ":" << httpListeners.first << "\" ...\n";
			listener.second->initializeContext();
			logger.info << "Initialization of objects for http-listener \"" << listener.first << ":" << httpListeners.first << "\" done.\n";
		}
	}

	/* ********************************************* *
	 * create broker clients for all message-brokers *
	 * ********************************************* */
	for(auto& entry : messageBrokerById) {
		messaging::proxy::Client& messageBroker = entry.second;

		esl::object::Interface::Object* object = new MessageBrokerObject(messageBroker, *this);
		messageBroker.object.reset(object);
		//std::unique_ptr<esl::object::Interface::Object> object(new BokerObject(messageBroker, *this));
		//messageBroker.object = std::move(object);

		std::unique_ptr<esl::messaging::Client> client(new esl::messaging::Client(messageBroker.brokers, messageBroker.settings, messageBroker.implementation));
		client->addObjectFactory("", [object](const esl::messaging::MessageContext&){ return object; });
		//client->addObjectFactory("", [this](const esl::messaging::MessageContext&){ return &this->engineObject; });

		messageBroker.client = std::move(client);
//		std::unique_ptr<esl::object::Interface::Object> object;

	}

	/* ******************************************************* *
	 * create sockets for all http-servers and add             *
	 * certificates to socket if http-server is used for https *
	 * ******************************************************* */
	for(auto& httpServer : httpServerById) {
		std::unique_ptr<esl::http::server::Socket> socket(new esl::http::server::Socket(httpServer.second.port, createRequestHandler, httpServer.second.settings, httpServer.second.implementation));
		socket->addObjectFactory("", [this](const esl::http::server::RequestContext&){ return &this->httpObject; });

		std::set<std::string> hostnames;

		if(httpServer.second.isHttps) {
			for(auto& httpListeners : httpListenersByPort) {
				if(httpListeners.first != httpServer.second.port) {
					continue;
				}

				for(auto& listener : httpListeners.second.listenerByHostname) {
					hostnames.insert(listener.first);
				}
			}
		}

		for(const auto& hostname : hostnames) {
			auto certIter = certsByHostname.find(hostname);
			if(certIter == std::end(certsByHostname)) {
				throw std::runtime_error("No certificate available for hostname '" + hostname + "'.");
			}
			socket->addTLSHost(hostname, certIter->second.first, certIter->second.second);
		}

		httpServer.second.socket = std::move(socket);
	}
/*
	std::string hostname = requestContext.getRequest().getHostName();
	logger.debug << "Lookup ListernerByHostname(" << hostname << ")\"\n";
	auto iterListenerByHostname = iterListenerByPort->second.listenerByHostname.find(hostname);

	while(iterListenerByHostname == std::end(iterListenerByPort->second.listenerByHostname)) {
		std::string::size_type pos = hostname.find_first_of('.');
		if(pos == std::string::npos) {
			logger.debug << "Lookup ListernerByHostname(*)\"\n";
			iterListenerByHostname = iterListenerByPort->second.listenerByHostname.find("*");
			break;
		}
		hostname = hostname.substr(pos+1);
		logger.debug << "Lookup ListernerByHostname(*." << hostname << ")\"\n";
		iterListenerByHostname = iterListenerByPort->second.listenerByHostname.find("*." + hostname);
	}

	if(iterListenerByHostname == std::end(iterListenerByPort->second.listenerByHostname)) {
		logger.debug << "Not found\n";
		return nullptr;
	}
 */
	auto stopFunction = [this]() { stop(); };
	esl::system::SignalHandler::install(esl::system::SignalHandler::SignalType::interrupt, stopFunction);
	esl::system::SignalHandler::install(esl::system::SignalHandler::SignalType::terminate, stopFunction);
	esl::system::SignalHandler::install(esl::system::SignalHandler::SignalType::pipe, stopFunction);

	logger.debug << "Start message broker consumers.\n";
	for(const auto& entry : messageBrokerById) {
		const messaging::proxy::Client& messageBroker = entry.second;
		std::set<std::string> queues;

		logger.debug << "-> Start consumer of message broker \"" << messageBroker.id << "\" for queues:\n";
		for(const auto& queue : messageBroker.handlerByQueueName) {
			logger.debug << "   - \"" << queue.first << "\"\n";
			queues.insert(queue.first);
		}

		esl::messaging::Consumer& consumer = messageBroker.client->getConsumer();
		consumer.start(queues, createMessageHandler, messageBroker.threads);
	}

	logger.debug << "Start http/https servers.\n";
	for(const auto& httpServer : httpServerById) {
		logger.debug << "-> Start http/https server \"" << httpServer.first << "\".\n";
		httpServer.second.socket->listen();
	}

	bool rc = messageTimer.run();

	esl::system::SignalHandler::remove(esl::system::SignalHandler::SignalType::pipe, stopFunction);
	esl::system::SignalHandler::remove(esl::system::SignalHandler::SignalType::terminate, stopFunction);
	esl::system::SignalHandler::remove(esl::system::SignalHandler::SignalType::interrupt, stopFunction);

	logger.debug << "Stop message broker consumers.\n";
	for(const auto& messageBroker : messageBrokerById) {
		logger.debug << "-> Stop consumer of message broker \"" << messageBroker.first << "\"\n";
		messageBroker.second.get().client->getConsumer().stop();
	}

	logger.debug << "Stop http/https servers.\n";
	for(const auto& httpServer : httpServerById) {
		logger.debug << "-> Stop http/https server \"" << httpServer.first << "\".\n";
		httpServer.second.socket->release();
	}

	logger.debug << "Wait for finished message broker consumers.\n";
	for(const auto& messageBroker : messageBrokerById) {
		logger.debug << "-> Waiting for consumer of message broker \"" << messageBroker.first << "\"\n";
		messageBroker.second.get().client->getConsumer().wait(0);
		logger.debug << "-> Waiting done.\n";
	}

	return rc;
}

bool Engine::runCGI() {
	return true;
}

void Engine::stop() {
	logger.info << "stopping engine\n";
	messageTimer.stop();
}

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

void Engine::addMessageBroker(const std::string& id, const std::string& brokers, std::uint16_t threads, const std::vector<std::pair<std::string, std::string>>& settings, const std::string& implementation) {
	logger.trace << "Adding message broker (implementation=\"" << implementation << "\") with id=\"" << id << "\" for broker " << brokers << "\"\n";

	auto iterMessageBroker = messageBrokerById.find(id);
	if(iterMessageBroker != std::end(messageBrokerById)) {
        throw std::runtime_error("There are multiple message brokers defined with same id \"" + id + "\".");
	}

	messaging::proxy::Client* messageBroker = new messaging::proxy::Client(id, brokers, threads, settings, implementation);
	std::unique_ptr<esl::object::Interface::Object> messageBrokerObject(messageBroker);
	addObject(id, std::move(messageBrokerObject));

	messageBrokerById.insert(std::make_pair(id, std::ref(*messageBroker)));
}

messaging::Listener& Engine::addMessageListener(const std::string& refId, bool inheritObjects) {
	auto iterMessageBroker = messageBrokerById.find(refId);
	if(iterMessageBroker == std::end(messageBrokerById)) {
        throw std::runtime_error("message-listener has an unknown refId \"" + refId + "\".");
	}

	messaging::Listener* messageListener = new messaging::Listener(*this, iterMessageBroker->second.get().handlerByQueueName, inheritObjects);
	iterMessageBroker->second.get().listeners.push_back(std::unique_ptr<messaging::Listener>(messageListener));

	return *messageListener;
}

void Engine::addHttpServer(const std::string& id, std::uint16_t port, bool isHttps, const std::vector<std::pair<std::string, std::string>>& settings, const std::string& implementation) {
	if(isHttps) {
		logger.trace << "Adding HTTPS server (implementation=\"" << implementation << "\") with id=\"" << id << "\" at port " << port << "\"\n";
	}
	else {
		logger.trace << "Adding HTTP server (implementation=\"" << implementation << "\") with id=\"" << id << "\" at port " << port << "\"\n";
	}

	if(listeningPorts.count(port) > 0) {
        throw std::runtime_error("There are multiple servers listening for port " + std::to_string(port) + ".");
	}
	listeningPorts.insert(port);

	auto iterHttpServer = httpServerById.find(id);
	if(iterHttpServer != std::end(httpServerById)) {
        throw std::runtime_error("There are multiple http servers defined with same id \"" + id + "\".");
	}

	httpServerById.insert(std::make_pair(id, HttpServer(id, port, isHttps, settings, implementation)));
}

http::Listener& Engine::addHttpListener(const std::string& hostname, const std::string& refId, bool inheritObjects) {
	auto iterHttpServer = httpServerById.find(refId);
	if(iterHttpServer == std::end(httpServerById)) {
        throw std::runtime_error("http-listener has an unknown refId \"" + refId + "\".");
	}

	HttpListeners& httpListeners = httpListenersByPort[iterHttpServer->second.port];
	std::unique_ptr<http::Listener>& httpListener = httpListeners.listenerByHostname[hostname];
	if(!httpListener) {
		httpListener.reset(new http::Listener(*this, inheritObjects));
	}

	return *httpListener;
}

void Engine::dumpTree(std::size_t depth) const {
	for(std::size_t i=0; i<depth; ++i) {
		logger.info << "|   ";
	}
	logger.info << "+-> Engine\n";

	++depth;
	BaseContext::dumpTree(depth);

	dumpTreeMessageBrokers(depth);
	dumpTreeHttpServers(depth, false);
	dumpTreeHttpServers(depth, true);
}

void Engine::dumpTreeMessageBrokers(std::size_t depth) const {
	for(std::size_t i=0; i<depth; ++i) {
		logger.info << "|   ";
	}
	logger.info << "+-> Messaging-Servers\n";
	for(const auto& entry: messageBrokerById) {
		const messaging::proxy::Client& messageBroker = entry.second;

		for(std::size_t i=0; i<depth; ++i) {
			logger.info << "|   ";
		}
		logger.info << "|   +-> ID: \"" << messageBroker.id << "\"\n";
		for(std::size_t i=0; i<depth; ++i) {
			logger.info << "|   ";
		}
		logger.info << "|   |   Implementation: \"" << messageBroker.implementation << "\"\n";
		for(std::size_t i=0; i<depth; ++i) {
			logger.info << "|   ";
		}
		logger.info << "|   |   Threads: " << messageBroker.threads << "\n";

		for(const auto& messageListener : messageBroker.listeners) {
			/*
			for(std::size_t i=0; i<depth; ++i) {
				logger.info << "|   ";
			}
			logger.info << "|   |   +-> Hostname: \"" << httpListener.first << "\"\n";
			*/
			messageListener->dumpTree(depth + 3);
		}
	}

}

void Engine::dumpTreeHttpServers(std::size_t depth, bool isHttps) const {
	for(std::size_t i=0; i<depth; ++i) {
		logger.info << "|   ";
	}
	if(isHttps) {
		logger.info << "+-> HTTPS Servers\n";
	}
	else {
		logger.info << "+-> HTTP Servers\n";
	}
	for(const auto& httpServer: httpServerById) {
		if(httpServer.second.isHttps != isHttps) {
			continue;
		}

		for(std::size_t i=0; i<depth; ++i) {
			logger.info << "|   ";
		}
		logger.info << "|   +-> ID: \"" << httpServer.first << "\"\n";
		for(std::size_t i=0; i<depth; ++i) {
			logger.info << "|   ";
		}
		logger.info << "|   |   Implementation: \"" << httpServer.second.implementation << "\"\n";
		for(std::size_t i=0; i<depth; ++i) {
			logger.info << "|   ";
		}
		logger.info << "|   |   Port: " << httpServer.second.port << "\n";

		auto iterHttpListeners = httpListenersByPort.find(httpServer.second.port);
		if(iterHttpListeners != std::end(httpListenersByPort)) {
			for(const auto& httpListener : iterHttpListeners->second.listenerByHostname) {
				for(std::size_t i=0; i<depth; ++i) {
					logger.info << "|   ";
				}
				logger.info << "|   |   +-> Hostname: \"" << httpListener.first << "\"\n";
				httpListener.second->dumpTree(depth + 3);
			}
		}
	}
}

std::unique_ptr<esl::utility::Consumer> Engine::createMessageHandler(esl::messaging::MessageContext& baseMessageContext) {
	/* Access log */
	logger.info << "Message for queueName \"" << baseMessageContext.getMessage().getId() << "\"\n";

	MessageBrokerObject* messageObject = dynamic_cast<MessageBrokerObject*>(baseMessageContext.findObject(""));
	if(!messageObject) {
		logger.warn << "MessageObject object not found\n";
		return nullptr;
	}

	messaging::proxy::Client& messageBroker = messageObject->getMessageBroker();
	auto iter = messageBroker.handlerByQueueName.find(baseMessageContext.getMessage().getId());

	if(iter == std::end(messageBroker.handlerByQueueName) || iter->second.empty()) {
		logger.warn << "No message handler defined for incoming message on queue \"" << baseMessageContext.getMessage().getId() << "\"\n";
		return nullptr;
	}

	return messaging::MessageHandler::create(baseMessageContext, iter->second);
}

std::unique_ptr<esl::http::server::requesthandler::Interface::RequestHandler> Engine::createRequestHandler(esl::http::server::RequestContext& requestContext) {
	/* Access log */
	logger.info << "Request for hostname " << requestContext.getRequest().getHostName() << ": " << requestContext.getRequest().getMethod() << " \"" << requestContext.getRequest().getPath() << "\" received from " << requestContext.getRequest().getRemoteAddress() << "\n";

	HttpObject* httpObject = dynamic_cast<HttpObject*>(requestContext.findObject(""));
	if(!httpObject) {
		ExceptionHandler exceptionHandler;

		exceptionHandler.setShowException(false);
		exceptionHandler.setShowStacktrace(false);
		//esl::http::server::exception::StatusCode e(500, "Engine object not found");
		//exceptionHandler.setMessage(e);
		exceptionHandler.call([]() {
			throw esl::http::server::exception::StatusCode(500, "Engine object not found");
		});
		exceptionHandler.dump(logger.error);
		exceptionHandler.dump(requestContext.getConnection());
		return std::unique_ptr<esl::http::server::requesthandler::Interface::RequestHandler>(new esl::http::server::requesthandler::Interface::RequestHandler);
		/*
		logger.error << "Engine object not found\n";

		return nullptr;
		*/
	}

	Engine& engine = httpObject->getEngine();

	http::Listener* listener = engine.getHttpListener(requestContext);

	if(listener == nullptr) {
		/* Error log */
		logger.info << "ERROR: No listener available for invalid request \"" << requestContext.getRequest().getPath() << "\"\n";

		//esl::http::server::exception::StatusCode e(404);
		ExceptionHandler exceptionHandler;

		exceptionHandler.setShowException(false);
		exceptionHandler.setShowStacktrace(false);
		//exceptionHandler.setMessage(esl::http::server::exception::StatusCode(500));
		exceptionHandler.call([]() {
			throw esl::http::server::exception::StatusCode(500);
		});
		exceptionHandler.dump(requestContext.getConnection());

		return std::unique_ptr<esl::http::server::requesthandler::Interface::RequestHandler>(new esl::http::server::requesthandler::Interface::RequestHandler);
	}
	return listener->createRequestHandler(requestContext);
}

http::Listener* Engine::getHttpListener(esl::http::server::RequestContext& requestContext) const {
	logger.debug << "Lookup ListernerByPort(" << requestContext.getRequest().getHostPort() << ")\"\n";
	auto iterListenerByPort = httpListenersByPort.find(requestContext.getRequest().getHostPort());
	if(iterListenerByPort == std::end(httpListenersByPort)) {
		logger.debug << "Not found\n";
		return nullptr;
	}
	logger.debug << "Found\n";

	std::string hostname = requestContext.getRequest().getHostName();
	logger.debug << "Lookup ListernerByHostname(" << hostname << ")\"\n";
	auto iterListenerByHostname = iterListenerByPort->second.listenerByHostname.find(hostname);

	while(iterListenerByHostname == std::end(iterListenerByPort->second.listenerByHostname)) {
		std::string::size_type pos = hostname.find_first_of('.');
		if(pos == std::string::npos) {
			logger.debug << "Lookup ListernerByHostname(*)\"\n";
			iterListenerByHostname = iterListenerByPort->second.listenerByHostname.find("*");
			break;
		}
		hostname = hostname.substr(pos+1);
		logger.debug << "Lookup ListernerByHostname(*." << hostname << ")\"\n";
		iterListenerByHostname = iterListenerByPort->second.listenerByHostname.find("*." + hostname);
	}

	if(iterListenerByHostname == std::end(iterListenerByPort->second.listenerByHostname)) {
		logger.debug << "Not found\n";
		return nullptr;
	}

	logger.debug << "Found\n";
	return iterListenerByHostname->second.get();
}


Engine::HttpServer::HttpServer(const std::string& aId, std::uint16_t aPort, bool aIsHttps, const std::vector<std::pair<std::string, std::string>>& aSettings, const std::string& aImplementation)
: id(aId),
  port(aPort),
  isHttps(aIsHttps),
  implementation(aImplementation)
{
	for(const auto& setting : aSettings) {
		settings.addSetting(setting.first, setting.second);
	}
}

} /* namespace engine */
} /* namespace jerry */
