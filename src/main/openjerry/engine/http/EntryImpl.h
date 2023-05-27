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

#ifndef OPENJERRY_ENGINE_HTTP_ENTRYIMPL_H_
#define OPENJERRY_ENGINE_HTTP_ENTRYIMPL_H_

#include <openjerry/engine/http/Entry.h>
#include <openjerry/engine/http/Context.h>
#include <openjerry/engine/http/Endpoint.h>
#include <openjerry/engine/http/Host.h>
#include <openjerry/engine/http/RequestContext.h>

#include <esl/com/http/server/RequestHandler.h>
#include <esl/io/Input.h>
#include <esl/processing/Procedure.h>

#include <string>
#include <memory>

namespace openjerry {
namespace engine {
namespace http {


class EntryImpl : public Entry {
public:
	EntryImpl(std::unique_ptr<esl::processing::Procedure> procedure);
	EntryImpl(esl::processing::Procedure& refProcedure);
	EntryImpl(std::unique_ptr<Context> context);
	EntryImpl(Context& refContext);
	EntryImpl(std::unique_ptr<Endpoint> endpoint);
	EntryImpl(std::unique_ptr<Host> host);
	EntryImpl(std::unique_ptr<esl::com::http::server::RequestHandler> requestHandler);

	void initializeContext(Context& ownerContext) override;
	void dumpTree(std::size_t depth) const override;
	esl::io::Input accept(RequestContext& requestContext) override;

private:
	std::unique_ptr<esl::processing::Procedure> procedure;
	esl::processing::Procedure* refProcedure = nullptr;

	std::unique_ptr<Context> context;
	Context* refContext = nullptr;

	std::unique_ptr<Endpoint> endpoint;
	std::unique_ptr<Host> host;

	std::unique_ptr<esl::com::http::server::RequestHandler> requestHandler;
};


} /* namespace http */
} /* namespace engine */
} /* namespace openjerry */

#endif /* OPENJERRY_ENGINE_HTTP_ENTRYIMPL_H_ */
