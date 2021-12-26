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

#ifndef JERRY_ENGINE_ENGINE_H_
#define JERRY_ENGINE_ENGINE_H_

#include <jerry/engine/Entry.h>
#include <jerry/engine/basic/server/Socket.h>
#include <jerry/engine/basic/server/Context.h>
#include <jerry/engine/http/server/Socket.h>
#include <jerry/engine/http/server/Context.h>
#include <jerry/engine/ObjectContext.h>

#include <esl/object/Interface.h>
#include <esl/processing/daemon/Interface.h>

#include <cstdint>
#include <set>
#include <map>
#include <vector>
#include <string>
#include <utility>
#include <thread>
#include <functional>
#include <memory>

namespace jerry {
namespace engine {

class Engine : public ObjectContext {
public:
	Engine();

	void addCertificate(const std::string& hostname, std::vector<unsigned char> key, std::vector<unsigned char> certificate);
	void addCertificate(const std::string& hostname, const std::string& keyFile, const std::string& certificateFile);
	const std::map<std::string, std::pair<std::vector<unsigned char>, std::vector<unsigned char>>>& getCertificates() const noexcept;
	const std::pair<std::vector<unsigned char>, std::vector<unsigned char>>* getCertsByHostname(const std::string& hostname) const;

	basic::server::Context& addBasicServer(const std::vector<std::pair<std::string, std::string>>& settings, const std::string& implementation);
	http::server::Context& addHttpServer(bool isHttps, const std::vector<std::pair<std::string, std::string>>& settings, const std::string& implementation);
	void addDaemon(const std::vector<std::pair<std::string, std::string>>& settings, const std::string& implementation);

	void addBasicClient(const std::string& id, const std::vector<std::pair<std::string, std::string>>& settings, const std::string& implementation);
	void addHttpClient(const std::string& id, const std::string& url, const std::vector<std::pair<std::string, std::string>>& settings, const std::string& implementation);

	const std::vector<std::unique_ptr<basic::server::Socket>>& getBasicServers() const;
	const std::vector<std::unique_ptr<http::server::Socket>>& getHttpServers() const;
	const std::vector<std::unique_ptr<esl::processing::daemon::Interface::Daemon>>& getDaemons() const;

	void initializeContext() override;
	void dumpTree(std::size_t depth) const override;

private:
	void dumpTreeBasicServers(std::size_t depth) const;
	void dumpTreeHttpServers(std::size_t depth) const;
	void dumpTreeDaemons(std::size_t depth) const;

	std::map<std::string, std::pair<std::vector<unsigned char>, std::vector<unsigned char>>> certsByHostname;

	std::vector<std::unique_ptr<basic::server::Socket>> basicServers;
	std::vector<std::unique_ptr<http::server::Socket>> httpServers;
	std::vector<std::unique_ptr<esl::processing::daemon::Interface::Daemon>> daemons;

};

} /* namespace engine */
} /* namespace jerry */

#endif /* JERRY_ENGINE_ENGINE_H_ */
