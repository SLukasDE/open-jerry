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

#ifndef JERRY_ENGINE_HTTP_CONTEXT_H_
#define JERRY_ENGINE_HTTP_CONTEXT_H_

#include <jerry/engine/http/Entry.h>
#include <jerry/engine/http/Document.h>
#include <jerry/engine/ObjectContext.h>
#include <jerry/engine/ProcessRegistry.h>

#include <esl/com/http/server/RequestHandler.h>
#include <esl/io/Input.h>
#include <esl/processing/Procedure.h>

#include <string>
#include <map>
#include <vector>
#include <memory>

namespace jerry {
namespace engine {
namespace http {

class RequestContext;
class Endpoint;
class Host;

class Context : public ObjectContext {
public:
	enum OptionalBool {
		obEmpty,
		obTrue,
		obFalse
	};

	Context(ProcessRegistry* processRegistry, bool followParentOnFind = true);

	void setParent(Context* context);
	const Context* getParent() const;

	void addProcedure(std::unique_ptr<esl::processing::Procedure> procedure);
	void addProcedure(const std::string& refId);

	void addContext(const std::string& refId);
	void addContext(std::unique_ptr<Context> context);
	void addEndpoint(std::unique_ptr<Endpoint> endpoint);
	void addHost(std::unique_ptr<Host> host);
	void addRequestHandler(std::unique_ptr<esl::com::http::server::RequestHandler> requestHandler);

	void setShowException(OptionalBool showException);
	bool getShowException() const;

	void setShowStacktrace(OptionalBool showStacktrace);
	bool getShowStacktrace() const;

	void setInheritErrorDocuments(bool inheritErrorDocuments);
	bool getInheritErrorDocuments() const;

	void addErrorDocument(unsigned short statusCode, const std::string& path, bool parse);
	const Document* findErrorDocument(unsigned short statusCode) const;

	void addHeader(std::string key, std::string value);
	const std::map<std::string, std::string>& getHeaders() const;
	const std::map<std::string, std::string>& getEffectiveHeaders() const;

	void initializeContext() override;
	void dumpTree(std::size_t depth) const override;
	esl::io::Input accept(RequestContext& requestContext);

private:
	std::vector<std::unique_ptr<Entry>> entries;

	Context* parent = nullptr;
	bool followParentOnFind = true;

	OptionalBool showException = obEmpty;
	OptionalBool showStacktrace = obEmpty;
	bool inheritErrorDocuments = true;

	/* maps Status Code to Error-Doc-Path and Flag, if content has to be parsed */
	std::map<unsigned short, Document> errorDocuments;

	std::map<std::string, std::string> headers;
	std::map<std::string, std::string> headersEffective;
};


} /* namespace http */
} /* namespace engine */
} /* namespace jerry */

#endif /* JERRY_ENGINE_HTTP_CONTEXT_H_ */
