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

#ifndef JERRY_ENGINE_BASIC_SERVER_ENTRYIMPL_H_
#define JERRY_ENGINE_BASIC_SERVER_ENTRYIMPL_H_

#include <jerry/engine/basic/server/Entry.h>
#include <jerry/engine/basic/server/Context.h>
#include <jerry/engine/basic/server/RequestContext.h>

#include <esl/io/Input.h>
#include <esl/com/basic/server/requesthandler/Interface.h>

#include <string>
#include <memory>

namespace jerry {
namespace engine {
namespace basic {
namespace server {

class EntryImpl : public Entry {
public:
	EntryImpl(std::unique_ptr<Context> context);
	EntryImpl(Context& refContext);
	EntryImpl(std::unique_ptr<esl::com::basic::server::requesthandler::Interface::RequestHandler> requestHandler);

	void initializeContext(Context& ownerContext) override;
	std::set<std::string> getNotifiers() const override;
	void dumpTree(std::size_t depth) const override;
	esl::io::Input accept(RequestContext& requestContext) override;

private:
	std::unique_ptr<Context> context;
	Context* refContext = nullptr;
	std::unique_ptr<esl::com::basic::server::requesthandler::Interface::RequestHandler> requestHandler;
};

} /* namespace server */
} /* namespace basic */
} /* namespace engine */
} /* namespace jerry */

#endif /* JERRY_ENGINE_BASIC_SERVER_ENTRYIMPL_H_ */
