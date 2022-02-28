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

#ifndef JERRY_PROCEDURE_H_
#define JERRY_PROCEDURE_H_

#include <jerry/config/Engine.h>
#include <jerry/engine/Engine.h>

#include <esl/processing/procedure/Interface.h>
#include <esl/object/ObjectContext.h>
#include <esl/module/Interface.h>
#include <esl/logging/appender/Interface.h>
#include <esl/system/Interface.h>

//#include <atomic>
#include <set>
#include <mutex>
#include <condition_variable>
#include <memory>

namespace jerry {

class Procedure final : public esl::processing::procedure::Interface::Procedure {
public:
	static inline const char* getImplementation() {
		return "jerry";
	}

	static std::unique_ptr<esl::processing::procedure::Interface::Procedure> create(const esl::module::Interface::Settings& settings);

	Procedure(const esl::module::Interface::Settings& settings);

	void procedureRun(esl::object::ObjectContext& objectContext) override;
	void procedureCancel() override;

private:
	std::string config;
	std::string configFile;
	int terminateCounter = -1;
	std::set<esl::system::Interface::SignalType> stopSignals;
	bool verbose = false;

	std::unique_ptr<jerry::config::Engine> configEngine;
	std::unique_ptr<jerry::engine::Engine> jEngine;

	std::mutex processCountMutex;
	unsigned int processCount = 0;
	std::condition_variable processCountCondVar;

	std::mutex signalThreadMutex;
	std::condition_variable signalThreadCondVar;

	std::mutex signalThreadRunningMutex;

	unsigned int getProcessCount();
	void processRegister();
	void processUnregister();

	static void flushLogAppender(const std::string& id, esl::logging::appender::Interface::Appender& appender);

};

} /* namespace jerry */

#endif /* JERRY_PROCEDURE_H_ */
