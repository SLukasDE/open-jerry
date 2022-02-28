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

#ifndef JERRY_ENGINE_DAEMON_H_
#define JERRY_ENGINE_DAEMON_H_

#include <esl/processing/procedure/Interface.h>
#include <esl/object/ObjectContext.h>

#include <functional>
#include <mutex>
#include <memory>

namespace jerry {
namespace engine {

class Daemon {
public:
	Daemon(std::unique_ptr<esl::processing::procedure::Interface::Procedure> procedure);
	~Daemon();

	void initializeContext(esl::object::ObjectContext& objectContext);

	void start(std::function<void()> onReleasedHandler);
	void release();

private:
	std::unique_ptr<esl::processing::procedure::Interface::Procedure> procedure;
	std::function<void()> onReleasedHandler;

	std::mutex runningMutex;
};

} /* namespace engine */
} /* namespace jerry */

#endif /* JERRY_ENGINE_DAEMON_H_ */
