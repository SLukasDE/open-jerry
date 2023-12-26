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

#ifndef OPENJERRY_ENGINE_MAIN_CONTEXT_H_
#define OPENJERRY_ENGINE_MAIN_CONTEXT_H_

#include <openjerry/engine/ObjectContext.h>
#include <openjerry/engine/http/Server.h>
#include <openjerry/engine/main/Entry.h>
#include <openjerry/engine/procedure/Context.h>
#include <openjerry/engine/ProcessRegistry.h>

#include <esl/object/Object.h>
#include <esl/object/Procedure.h>
#include <esl/system/Signal.h>
#include <esl/utility/Signal.h>
#include <esl/monitoring/Appender.h>

#include <atomic>
#include <condition_variable>
#include <cstdint>
#include <functional>
#include <map>
#include <memory>
#include <mutex>
#include <set>
#include <string>
#include <thread>
#include <utility>
#include <vector>

namespace openjerry {
namespace engine {
namespace main {

class Context final : public ObjectContext, public ProcessRegistry, public esl::object::Procedure {
public:
	static std::unique_ptr<esl::object::Procedure> create(const std::vector<std::pair<std::string, std::string>>& settings);

	Context(const std::vector<std::pair<std::string, std::string>>& settings);

	/* deprecated certificate stuff */
	//const std::map<std::string, std::pair<std::vector<unsigned char>, std::vector<unsigned char>>>& getCertificates() const noexcept;
	//const std::pair<std::vector<unsigned char>, std::vector<unsigned char>>* getCertsByHostname(const std::string& hostname) const;

	/* context specific methods */
	void addProcedure(std::unique_ptr<esl::object::Procedure> procedure);
	void addProcedure(const std::string& refId);

	void addHttpServer(std::unique_ptr<http::Server> server);

	void addProcedureContext(std::unique_ptr<procedure::Context> procedureContext);
	void addProcedureContext(const std::string& refId);

	/* specializations of ObjectContext */
	void initializeContext() override;
	void dumpTree(std::size_t depth) const override;

	void procedureRun(esl::object::Context& objectContext) override;
	void procedureCancel() override;

	void setProcessRegistry(ProcessRegistry* processRegistry) override;

	/* specializations of ProcessRegistry */
	void processRegister(esl::object::Procedure& procedureRunning) override;
	void processUnregister(esl::object::Procedure& procedureRunning) override;

	void addObject(const std::string& id, std::unique_ptr<esl::object::Object> object) override;

private:
	std::atomic<int> terminateCounter{-1};
	std::set<esl::utility::Signal> stopSignals;
	bool verbose = false;

	bool catchException = true;
	bool dumpException = true;
	bool hasExceptionReturnCode = false;
	int exceptionReturnCode = -1;

	std::unique_ptr<esl::system::Signal> signal;

	std::vector<std::unique_ptr<Entry>> entries;

	std::mutex proceduresRunningMutex;
	std::set<esl::object::Procedure*> proceduresRunning;
	bool proceduresRunningCancel = false;
	std::condition_variable proceduresRunningCondVar;

	std::mutex signalThreadMutex;
	std::condition_variable signalThreadCondVar;

	std::mutex signalThreadRunningMutex;

	unsigned int getProceduresRunningCount();
};

} /* namespace main */
} /* namespace engine */
} /* namespace openjerry */

#endif /* OPENJERRY_ENGINE_MAIN_CONTEXT_H_ */
