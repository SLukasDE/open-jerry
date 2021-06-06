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

#include <jerry/engine/basic/server/Listener.h>
#include <jerry/engine/basic/server/Writer.h>
#include <jerry/Logger.h>

#include <stdexcept>

namespace jerry {
namespace engine {
namespace basic {
namespace server {

namespace {
Logger logger("jerry::engine::messaging::server::Listener");
}

Listener::Listener(esl::object::ObjectContext& aEngineContext, bool inheritObjects, std::vector<std::string> aRefIds)
: Context(*this, nullptr, inheritObjects),
  engineContext(aEngineContext),
  refIds(std::move(aRefIds))
{ }

esl::object::Interface::Object* Listener::findHiddenObject(const std::string& id) const {
	esl::object::Interface::Object* object = Context::findLocalObject(id);

	if(object) {
		return object;
	}

	return engineContext.findObject<esl::object::Interface::Object>(id);
}

void Listener::dumpTree(std::size_t depth) const {
	for(const auto& refId : refIds) {
		for(std::size_t i=0; i<depth; ++i) {
			logger.info << "|   ";
		}
		logger.info << "Added to: \"" + refId + "\"\n";
	}

	Context::dumpTree(depth);
}

const std::vector<std::string>& Listener::getRefIds() const {
	return refIds;
}

esl::io::Input Listener::createRequestHandler(esl::com::basic::server::RequestContext& baseRequestContext) {
	std::unique_ptr<Writer> writer(new Writer(*this, baseRequestContext));

	/* *************************************************************************** *
	 * Delegate to Context::createRequestHandler(Writer& writer) -> esl::io::Input *
	 * Returns a valid input object if a request handler has been found.           *
	 * *************************************************************************** */
	return Context::createRequestHandler(writer);
}


} /* namespace server */
} /* namespace basic */
} /* namespace engine */
} /* namespace jerry */
