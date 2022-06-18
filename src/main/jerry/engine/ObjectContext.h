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

#ifndef JERRY_ENGINE_OBJECTCONTEXT_H_
#define JERRY_ENGINE_OBJECTCONTEXT_H_

#include <jerry/engine/ProcessRegistry.h>

#include <esl/object/Interface.h>
#include <esl/object/Context.h>

#include <functional>
#include <map>
#include <memory>
#include <set>
#include <string>
#include <vector>

namespace jerry {
namespace engine {

class ObjectContext : public esl::object::Context {
public:
	ObjectContext(const ObjectContext&) = delete;
	ObjectContext(ProcessRegistry* processRegistry);

	ObjectContext& operator=(const ObjectContext&) = delete;

	void setParent(esl::object::Context* objectContext);

	std::set<std::string> getObjectIds() const override;
	void addReference(const std::string& id, esl::object::Interface::Object& object);

	virtual void initializeContext();
	virtual void dumpTree(std::size_t depth) const;
	const std::map<std::string, std::reference_wrapper<esl::object::Interface::Object>>& getObjects() const;

	virtual void setProcessRegistry(ProcessRegistry* processRegistry);
	ProcessRegistry* getProcessRegistry();
	const ProcessRegistry* getProcessRegistry() const;

protected:
	esl::object::Interface::Object* findRawObject(const std::string& id) override;
	const esl::object::Interface::Object* findRawObject(const std::string& id) const override;
	void addRawObject(const std::string& id, std::unique_ptr<esl::object::Interface::Object> object) override; // final;

private:
	ProcessRegistry* processRegistry;
	esl::object::Context* parent = nullptr;
	std::map<std::string, std::unique_ptr<esl::object::Interface::Object>> objects;
	std::map<std::string, std::reference_wrapper<esl::object::Interface::Object>> objectRefsById;
};

} /* namespace engine */
} /* namespace jerry */

#endif /* JERRY_ENGINE_OBJECTCONTEXT_H_ */
