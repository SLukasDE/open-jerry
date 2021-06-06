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

#ifndef JERRY_BUILTIN_BASIC_DUMP_REQUESTHANDLER_H_
#define JERRY_BUILTIN_BASIC_DUMP_REQUESTHANDLER_H_

#include <jerry/builtin/basic/dump/Settings.h>

#include <esl/com/basic/server/RequestContext.h>
#include <esl/object/ObjectContext.h>
#include <esl/io/Input.h>
#include <esl/io/Consumer.h>
#include <esl/io/Reader.h>

#include <string>

namespace jerry {
namespace builtin {
namespace basic {
namespace dump {

class RequestHandler : public esl::io::Consumer {
public:
	static esl::io::Input createInput(esl::com::basic::server::RequestContext& requestContext);
	static const std::set<std::string>& getNotifiers(const esl::object::ObjectContext&);

	RequestHandler(const Settings& settings);

	/* return: true for every kind of success and get called again for more content data
	 *         false for failure or to get not called again
	 */
	bool consume(esl::io::Reader& reader) override;

private:
	const Settings& settings;
};

} /* namespace dump */
} /* namespace basic */
} /* namespace builtin */
} /* namespace jerry */

#endif /* JERRY_BUILTIN_BASIC_DUMP_REQUESTHANDLER_H_ */
