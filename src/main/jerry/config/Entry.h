/*
 * This file is part of Jerry application server.
 * Copyright (C) 2020 Sven Lukas
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

#ifndef JERRY_CONFIG_ENTRY_H_
#define JERRY_CONFIG_ENTRY_H_

#include <memory>
#include <ostream>

namespace jerry {
namespace config {

class RequestHandler;
class Context;
class Endpoint;

class Entry {
public:
	Entry() = default;
	Entry(const Entry&) = delete;
	Entry(Entry&& other);
	~Entry();

	Entry& operator=(const Entry&) = delete;
	Entry& operator=(Entry&& other);

	void save(std::ostream& oStream, std::size_t spaces) const;

	void setRequestHandler(std::unique_ptr<RequestHandler> requestHandler);
	RequestHandler* getRequestHandler() const;

	void setContext(std::unique_ptr<Context> context);
	Context* getContext() const;

	void setEndpoint(std::unique_ptr<Endpoint> endpoint);
	Endpoint* getEndpoint() const;

private:
	void doDelete();

	RequestHandler* requestHandler = nullptr;
	Context* context = nullptr;
	Endpoint* endpoint = nullptr;
};

} /* namespace config */
} /* namespace jerry */

#endif /* JERRY_CONFIG_ENTRY_H_ */
