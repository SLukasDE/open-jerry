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

#ifndef JERRY_ENGINE_HTTP_SERVER_ENDPOINT_H_
#define JERRY_ENGINE_HTTP_SERVER_ENDPOINT_H_

#include <jerry/engine/http/server/Context.h>
#include <jerry/engine/http/server/Document.h>

#include <string>
#include <vector>
#include <map>

namespace jerry {
namespace engine {
namespace http {
namespace server {

class Listener;

class Endpoint : public Context {
friend class Context;
public:
	enum OptionalBool {
		obEmpty,
		obTrue,
		obFalse
	};

	const std::vector<std::string>& getPathList() const;
	std::vector<std::string> getFullPathList() const;
	std::size_t getDepth() const;

	const Endpoint* getParentEndpoint() const;

	void setShowException(bool showException);
	bool getShowException() const;

	void setShowStacktrace(bool showStacktrace);
	bool getShowStacktrace() const;

	void setInheritErrorDocuments(bool inheritErrorDocuments);
	bool getInheritErrorDocuments() const;

	void addErrorDocument(unsigned short statusCode, const std::string& path, bool parse);
	const Document* findErrorDocument(unsigned short statusCode) const;

	void addHeader(std::string key, std::string value);
	const std::map<std::string, std::string>& getHeaders() const;

	void dumpTree(std::size_t depth) const override;

protected:
	Endpoint(Listener& listener, const Endpoint& parentEndpoint, const Context& parentContext, std::vector<std::string> pathList, bool inheritObjects);

	// only used by Listener
	Endpoint(Listener& listener, bool inheritObjects);

private:
	const Endpoint* parentEndpoint = nullptr;
	std::vector<std::string> pathList;
	std::size_t depth = 0;

	OptionalBool showException = obEmpty;
	OptionalBool showStacktrace = obEmpty;
	bool inheritErrorDocuments = true;

	/* maps Status Code to Error-Doc-Path and Flag, if content has to be parsed */
	std::map<unsigned short, Document> errorDocuments;

	std::map<std::string, std::string> headers;
};

} /* namespace server */
} /* namespace http */
} /* namespace engine */
} /* namespace jerry */

#endif /* JERRY_ENGINE_HTTP_SERVER_ENDPOINT_H_ */
