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

#include <jerry/URL.h>
#include <memory>

namespace jerry {

URL::URL(const std::string& aURL)  {
	const char *str = aURL.c_str();

	size_t pos = 0;
	const size_t len = aURL.size();

	for(NextFragment nextFragment = NextFragment::SCHEME; nextFragment != NextFragment::EMPTY;) {
		switch(nextFragment) {
		case NextFragment::SCHEME:
			nextFragment = parseScheme(pos, str, len);
			break;
		case NextFragment::HOSTNAME:
			nextFragment = parseHostname(pos, str, len);
			break;
		case NextFragment::PORT:
			nextFragment = parsePort(pos, str, len);
			break;
		case NextFragment::PATH:
			nextFragment = parsePath(pos, str, len);
			break;
		case NextFragment::QUERY:
			nextFragment = parseQuery(pos, str, len);
			break;
		case NextFragment::TAG:
			nextFragment = parseTag(pos, str, len);
			break;
		default:
			nextFragment = NextFragment::EMPTY;
			break;
		}
	}
}

const std::string& URL::getScheme() const {
	return scheme;
}

const std::string& URL::getHostname() const {
	return hostname;
}

const std::string& URL::getPort() const {
	return port;
}

const std::string& URL::getPath() const {
	return path;
}

const std::string& URL::getQuery() const {
	return query;
}

const std::string& URL::getTag() const {
	return tag;
}

URL::NextFragment URL::parseScheme(size_t& pos, const char* str, const size_t len) {
	size_t scheme_pos = pos;

	// remove preceding spaces.
	for(;pos<len && str[pos]==' '; pos++) {
	}
	if (str[pos] == '/') {
		scheme.clear();
		++pos;
		return NextFragment::PATH;
	}

	for(;pos<len && str[pos] != ':'; pos++) {
	}
	if(pos > len) {
		return NextFragment::EMPTY;
	}

	std::string tmpScheme = std::string(&str[scheme_pos], pos - scheme_pos);

	if(len-pos < 3) {
		return NextFragment::EMPTY;
	}
	if(str[pos++] != ':') {
		return NextFragment::EMPTY;
	}
	if(str[pos++] != '/') {
		return NextFragment::EMPTY;
	}
	if(str[pos++] != '/') {
		return NextFragment::EMPTY;
	}

	scheme = std::move(tmpScheme);
	return NextFragment::HOSTNAME;
}

URL::NextFragment URL::parseHostname(size_t& pos, const char* str, const size_t len) {
	size_t host_pos = pos;

	for(;pos<len && str[pos]!=':' && str[pos]!='/' && str[pos]!='?'; pos++) {
	}
	hostname = std::string(&str[host_pos], pos - host_pos);

	if (pos < len) {
		switch(str[pos]) {
		case '/':
			++pos;
			return NextFragment::PATH;
		case '?':
			++pos;
			return NextFragment::QUERY;
		case ':':
			++pos;
			return NextFragment::PORT;
		default:
			break;
		}
	}

	return NextFragment::EMPTY;
}

URL::NextFragment URL::parsePort(size_t& pos, const char* str, const size_t len) {
	size_t port_pos = pos;

	for(;pos<len && str[pos]!='/' && str[pos]!='?' ;pos++) {
	}
	port = std::string(&str[port_pos], pos - port_pos);

	if (pos < len) {
		switch(str[pos]) {
		case '/':
			++pos;
			return NextFragment::PATH;
		case '?':
			++pos;
			return NextFragment::QUERY;
		default:
			break;
		}
	}

	return NextFragment::EMPTY;
}

URL::NextFragment URL::parsePath(size_t& pos, const char* str, const size_t len) {
	size_t path_pos = pos;

	for(;pos<len && str[pos]!='?' && str[pos]!='#';pos++) {
	}
	path = std::string(&str[path_pos], pos - path_pos);

	if (pos < len) {
		switch(str[pos]) {
		case '?':
			++pos;
			return NextFragment::QUERY;
		case '#':
			++pos;
			return NextFragment::TAG;
		default:
			break;
		}
	}

	return NextFragment::EMPTY;
}

URL::NextFragment URL::parseQuery(size_t& pos, const char* str, const size_t len) {
	size_t param_pos = pos;

	for(;pos<len && str[pos]!='#';pos++) {
	}
	query = std::string(&str[param_pos], pos - param_pos);

	if (pos < len) {
		switch(str[pos]) {
		case '#':
			++pos;
			return NextFragment::TAG;
		default:
			break;
		}
	}

	return NextFragment::EMPTY;
}

URL::NextFragment URL::parseTag(size_t& pos, const char* str, const size_t len) {
	tag = std::string(&str[pos]);
	return NextFragment::EMPTY;
}

} /* namespace jerry */
