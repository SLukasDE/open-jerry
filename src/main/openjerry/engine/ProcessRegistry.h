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

#ifndef OPENJERRY_ENGINE_PROCESSREGISTRY_H_
#define OPENJERRY_ENGINE_PROCESSREGISTRY_H_

#include <esl/object/Procedure.h>

namespace openjerry {
namespace engine {

class ProcessRegistry {
public:
	virtual ~ProcessRegistry() = default;

	virtual void processRegister(esl::object::Procedure& procedureRunning) = 0;
	virtual void processUnregister(esl::object::Procedure& procedureRunning) = 0;
};

} /* namespace engine */
} /* namespace openjerry */

#endif /* OPENJERRY_ENGINE_PROCESSREGISTRY_H_ */
