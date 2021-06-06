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

#ifndef JERRY_ENGINE_HTTP_SERVER_WRITER_H_
#define JERRY_ENGINE_HTTP_SERVER_WRITER_H_

#include <jerry/engine/http/server/RequestContext.h>

#include <esl/io/Writer.h>
#include <esl/com/http/server/RequestContext.h>

#include <string>

namespace jerry {
namespace engine {
namespace http {
namespace server {

class Listener;

class Writer : public esl::io::Writer {
public:
	Writer(const Listener& listener, esl::com::http::server::RequestContext& baseRequestContext);

	RequestContext& getRequestContext();

	// if function is called with size=0, this signals that writing is done, so write will not be called anymore.
	// -> this can be used for cleanup stuff.
	std::size_t write(const void* data, std::size_t size) override;

	// returns consumable bytes to write.
	// npos is returned if available size is unknown.
	std::size_t getSizeWritable() const override;

private:
	RequestContext requestContext;
};

} /* namespace server */
} /* namespace http */
} /* namespace engine */
} /* namespace jerry */

#endif /* JERRY_ENGINE_HTTP_SERVER_WRITER_H_ */
