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

#ifndef JERRY_CONFIG_HTTP_ENTRY_H_
#define JERRY_CONFIG_HTTP_ENTRY_H_

#include <jerry/config/Object.h>
#include <jerry/config/Reference.h>

#include <memory>
#include <ostream>

#include <tinyxml2/tinyxml2.h>

namespace jerry {
namespace config {
namespace http {

class Endpoint;
class Context;
class RequestHandler;

class Entry {
public:
	enum Type {
		etNone,
		etObject,
		etReference,
		etEndpoint,
		etContext,
		etRequestHandler
	};

	Entry() = delete;
	Entry(const Entry&) = delete;
	Entry(Entry&& other);
	Entry(const tinyxml2::XMLElement& element);

	~Entry();

	Entry& operator=(const Entry&) = delete;
	Entry& operator=(Entry&& other);

	void save(std::ostream& oStream, std::size_t spaces) const;

	Type getType() const;

	Object& getObject() const;
	Reference& getReference() const;
	Endpoint& getEndpoint() const;
	Context& getContext() const;
	RequestHandler& getRequestHandler() const;

private:
	void doDelete();

	Type type = etNone;
	std::unique_ptr<Object> object;
	std::unique_ptr<Reference> reference;
	Endpoint* endpoint = nullptr;
	Context* context = nullptr;
	RequestHandler* requestHandler = nullptr;
};

} /* namespace http */
} /* namespace config */
} /* namespace jerry */

#endif /* JERRY_CONFIG_HTTP_ENTRY_H_ */
