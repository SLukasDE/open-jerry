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

#ifndef JERRY_ENGINE_HTTP_SERVER_CONTEXT_H_
#define JERRY_ENGINE_HTTP_SERVER_CONTEXT_H_

#include <jerry/engine/http/server/Entry.h>
#include <jerry/engine/http/server/Document.h>
#include <jerry/engine/ObjectContext.h>

#include <esl/io/Input.h>
#include <esl/module/Interface.h>

#include <string>
#include <map>
#include <vector>
#include <memory>

namespace jerry {
namespace engine {
namespace http {
namespace server {

class RequestContext;

class Context : public ObjectContext {
public:
	enum OptionalBool {
		obEmpty,
		obTrue,
		obFalse
	};

	Context(bool followParentOnFind = true);

	void setParent(Context* context);
	const Context* getParent() const;

	Context& addContext(const std::string& id, bool inheritObjects);
	void addContext(const std::string& refId);

	Endpoint& addEndpoint(const std::string& path, bool inheritObjects);

	void addRequestHandler(const std::string& implementation, const esl::module::Interface::Settings& settings);

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

	virtual esl::io::Input accept(RequestContext& requestContext);

private:
	std::vector<Entry> entries;

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

} /* namespace server */
} /* namespace http */
} /* namespace engine */
} /* namespace jerry */

#endif /* JERRY_ENGINE_HTTP_SERVER_CONTEXT_H_ */

#include <jerry/engine/http/server/Endpoint.h> // important for compiler to know size of Context (see Entry.h)
