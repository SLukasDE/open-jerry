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

#include <jerry/engine/basic/server/Socket.h>
#include <jerry/Logger.h>

#include <esl/com/basic/server/Socket.h>
#include <esl/object/Interface.h>
#include <esl/Stacktrace.h>

#include <stdexcept>

namespace jerry {
namespace engine {
namespace basic {
namespace server {

namespace {
Logger logger("jerry::engine::messaging::server::Socket");
}

Socket::Socket(esl::object::ObjectContext& aEngineContext, const std::string& aId, esl::com::basic::server::Interface::Socket& aSocket)
: engineContext(aEngineContext),
  socket(aSocket),
  id(aId)
{
	getSocket().addObjectFactory("", [this](const esl::com::basic::server::RequestContext&) {
		esl::object::Interface::Object* object = this;
		return object;
	});
}

Socket::Socket(esl::object::ObjectContext& aEngineContext, const std::string& aId,
		std::uint16_t aPort,
		const std::vector<std::pair<std::string, std::string>>& settings,
		const std::string& aImplementation)
: engineContext(aEngineContext),
  socketPtr(new esl::com::basic::server::Socket(aPort, esl::object::Properties(settings), aImplementation)),
  socket(*socketPtr),
  id(aId),
  implementation(aImplementation),
  port(aPort)
{
	getSocket().addObjectFactory("", [this](const esl::com::basic::server::RequestContext&) {
		esl::object::Interface::Object* object = this;
		return object;
	});
}

void Socket::addObjectFactory(const std::string& id, ObjectFactory objectFactory) {
	getSocket().addObjectFactory(id, objectFactory);
}

void Socket::listen(const std::set<std::string>& notifications, esl::com::basic::server::requesthandler::Interface::CreateInput createInput) {
	logger.warn << "listen 'listen' is not allowed.\n";
	//throw esl::addStacktrace(std::runtime_error("Calling 'listen' is not allowed."));

	//return false;
}

void Socket::release() {
	logger.warn << "Calling 'release' is not allowed.\n";
	//throw esl::addStacktrace(std::runtime_error("Calling 'release' is not allowed."));
}

bool Socket::wait(std::uint32_t ms) {
	logger.warn << "Calling 'wait' is not allowed.\n";
	//throw esl::addStacktrace(std::runtime_error("Calling 'release' is not allowed."));

	return false;
}

void Socket::dumpTree(std::size_t depth) const {
	for(std::size_t i=0; i<depth; ++i) {
		logger.info << "|   ";
	}
	logger.info << "+-> ID: \"" << getId() << "\"\n";
	++depth;

	for(std::size_t i=0; i<depth; ++i) {
		logger.info << "|   ";
	}
	logger.info << "Implementation: \"" << getImplementation() << "\"\n";

	for(std::size_t i=0; i<depth; ++i) {
		logger.info << "|   ";
	}
	logger.info << "Port: \"" << getPort() << "\"\n";

	for(std::size_t i=0; i<depth; ++i) {
		logger.info << "|   ";
	}
}

void Socket::addListener(Listener& aListener) {
	if(listener) {
		if(socketPtr) {
			throw esl::addStacktrace(std::runtime_error("There is already a listener connected to basic-sever \"" + getId() + "\"."));
		}
		else {
			throw esl::addStacktrace(std::runtime_error("There is already a listener connected to basic-broker \"" + getId() + "\"."));
		}
	}
	listener = &aListener;
}

esl::com::basic::server::Interface::Socket& Socket::getSocket() const noexcept {
	return socket;
}

const std::string& Socket::getId() const noexcept {
	if(!socketPtr) {
		throw esl::addStacktrace(std::runtime_error("Calling 'Socket::getId' is not allowed."));
	}
	return id;
}

const std::string& Socket::getImplementation() const noexcept {
	if(!socketPtr) {
		throw esl::addStacktrace(std::runtime_error("Calling 'Socket::getImplementation' is not allowed."));
	}
	return implementation;
}

std::uint16_t Socket::getPort() const noexcept {
	if(!socketPtr) {
		throw esl::addStacktrace(std::runtime_error("Calling 'Socket::getPort' is not allowed."));
	}
	return port;
}

std::set<std::string> Socket::getNotifier() const {
	std::set<std::string> notifier;

	if(listener) {
		std::set<std::string> tmpNotifier = listener->getNotifier();
		notifier.insert(tmpNotifier.begin(), tmpNotifier.end());
	}

	return notifier;
}

esl::io::Input Socket::createMessageHandler(esl::com::basic::server::RequestContext& baseRequestContext) {
	/* Access log */
	logger.info << "Incoming message with " << baseRequestContext.getRequest().getValues().size() << " meta properties:\"\n";
	for(std::size_t i=0; i<baseRequestContext.getRequest().getValues().size(); ++i) {
		logger.info << "- \"" << baseRequestContext.getRequest().getValues()[i].first << "\" = \"" << baseRequestContext.getRequest().getValues()[i].second << "\"\n";
	}

	Socket* socket = baseRequestContext.findObject<Socket>("");
	if(!socket) {
		logger.info << "ERROR: No basic-server or basic-broker available for request.\n";
		return esl::io::Input();
	}

	if(socket->socketPtr) {
		logger.debug << "Basic-server found with id " << socket->getId() << "\"\n";
	}
	else {
		logger.debug << "Basic-broker found with id " << socket->getId() << "\"\n";
	}

	if(!socket->listener) {
		logger.info << "No listener installed.\n";
		return esl::io::Input();
	}

	return socket->listener->createRequestHandler(baseRequestContext);
}

} /* namespace server */
} /* namespace basic */
} /* namespace engine */
} /* namespace jerry */
