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

#ifndef JERRY_URL_H_
#define JERRY_URL_H_

#include <iostream>

namespace jerry {

class URL {
public:
	URL(const std::string& url);

	const std::string& getScheme() const;
	const std::string& getHostname() const;
	const std::string& getPort() const;
	const std::string& getPath() const;
	const std::string& getQuery() const;
	const std::string& getTag() const;

private:
	enum class NextFragment {
		SCHEME,
		HOSTNAME,
		PORT,
		PATH,
		QUERY,
		TAG,
		EMPTY
	};
	NextFragment parseScheme(size_t& pos, const char* str, const size_t len);
	NextFragment parseHostname(size_t& pos, const char* str, const size_t len);
	NextFragment parsePort(size_t& pos, const char* str, const size_t len);
	NextFragment parsePath(size_t& pos, const char* str, const size_t len);
	NextFragment parseQuery(size_t& pos, const char* str, const size_t len);
	NextFragment parseTag(size_t& pos, const char* str, const size_t len);

	std::string scheme;
	std::string hostname;
	std::string port;
	std::string path;
	std::string query;
	std::string tag;
};

} /* namespace jerry */

#endif /* JERRY_URL_H_ */
