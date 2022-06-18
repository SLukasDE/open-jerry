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

#ifndef JERRY_ENGINE_MAIN_CONTEXT_H_
#define JERRY_ENGINE_MAIN_CONTEXT_H_

#include <jerry/engine/ObjectContext.h>
#include <jerry/engine/basic/Server.h>
#include <jerry/engine/http/Server.h>
#include <jerry/engine/main/Entry.h>
#include <jerry/engine/procedure/Context.h>
#include <jerry/engine/ProcessRegistry.h>

#include <esl/object/Interface.h>
#include <esl/object/Event.h>
#include <esl/processing/procedure/Interface.h>
#include <esl/system/signal/Signal.h>
#include <esl/utility/Signal.h>
#include <esl/logging/appender/Interface.h>

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

namespace jerry {
namespace engine {
namespace main {

class Context final : public ObjectContext, public ProcessRegistry, public esl::processing::procedure::Interface::Procedure { //, public esl::object::Event {
public:
	static std::unique_ptr<esl::processing::procedure::Interface::Procedure> create(const std::vector<std::pair<std::string, std::string>>& settings);

	Context(const std::vector<std::pair<std::string, std::string>>& settings);

	/* deprecated certificate stuff */
	void addCertificate(const std::string& hostname, std::vector<unsigned char> key, std::vector<unsigned char> certificate);
	void addCertificate(const std::string& hostname, const std::string& keyFile, const std::string& certificateFile);
	const std::map<std::string, std::pair<std::vector<unsigned char>, std::vector<unsigned char>>>& getCertificates() const noexcept;
	const std::pair<std::vector<unsigned char>, std::vector<unsigned char>>* getCertsByHostname(const std::string& hostname) const;

	/* context specific methods */
	//void addReference(const std::string& id, esl::object::Interface::Object& object);

	void addProcedure(std::unique_ptr<esl::processing::procedure::Interface::Procedure> procedure);
	void addProcedure(const std::string& refId);

	void addBasicServer(std::unique_ptr<basic::Server> server);
	void addHttpServer(std::unique_ptr<http::Server> server);

	void addProcedureContext(std::unique_ptr<procedure::Context> procedureContext);
	void addProcedureContext(const std::string& refId);

	/* specializations of ObjectContext */
	void initializeContext() override;
	void dumpTree(std::size_t depth) const override;

	void procedureRun(esl::object::Context& objectContext) override;
	void procedureCancel() override;

	//void onSignal(const esl::utility::Signal& signal);

	void setProcessRegistry(ProcessRegistry* processRegistry) override;

	/* specializations of ProcessRegistry */
	void processRegister(esl::processing::procedure::Interface::Procedure& procedureRunning) override;
	void processUnregister(esl::processing::procedure::Interface::Procedure& procedureRunning) override;

protected:
	void addRawObject(const std::string& id, std::unique_ptr<esl::object::Interface::Object> object) override;

private:
	std::atomic<int> terminateCounter{-1};
	std::set<esl::utility::Signal> stopSignals;
	bool verbose = false;

	bool catchException = true;
	bool dumpException = true;
	bool hasExceptionReturnCode = false;
	int exceptionReturnCode = -1;

	esl::system::signal::Signal signal;
	std::map<std::string, std::pair<std::vector<unsigned char>, std::vector<unsigned char>>> certsByHostname;

	std::vector<std::unique_ptr<Entry>> entries;

	std::mutex proceduresRunningMutex;
	std::set<esl::processing::procedure::Interface::Procedure*> proceduresRunning;
	bool proceduresRunningCancel = false;
	std::condition_variable proceduresRunningCondVar;

	std::mutex signalThreadMutex;
	std::condition_variable signalThreadCondVar;

	std::mutex signalThreadRunningMutex;

	unsigned int getProceduresRunningCount();
};

} /* namespace main */
} /* namespace engine */
} /* namespace jerry */

#endif /* JERRY_ENGINE_MAIN_CONTEXT_H_ */
