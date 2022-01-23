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

#include <esl/object/Interface.h>
#include <esl/object/ObjectContext.h>

#include <string>
#include <map>
#include <vector>
#include <memory>
#include <functional>

namespace jerry {
namespace engine {

class ObjectContext : public esl::object::ObjectContext {
public:
	ObjectContext(bool isGlobal = false);

	void setParent(esl::object::ObjectContext* objectContext);

	void addObject(const std::string& id, std::unique_ptr<esl::object::Interface::Object> object) override;
	void addReference(const std::string& id, const std::string& refId);


	//void initializeContext(esl::object::Interface::ObjectContext& objectContext) override;
	virtual void initializeContext();
	virtual void dumpTree(std::size_t depth) const;
	esl::object::Interface::Object* getObject() const;
	const std::map<std::string, std::reference_wrapper<esl::object::Interface::Object>>& getObjects() const;

protected:
	esl::object::Interface::Object* findRawObject(const std::string& id) override;
	const esl::object::Interface::Object* findRawObject(const std::string& id) const override;

private:
	bool isGlobal;
	esl::object::ObjectContext* parent = nullptr;
	std::map<std::string, std::unique_ptr<esl::object::Interface::Object>> objects;
	std::map<std::string, std::reference_wrapper<esl::object::Interface::Object>> objectRefsById;

	void addReference(const std::string& id, esl::object::Interface::Object& object);
};

} /* namespace engine */
} /* namespace jerry */

#endif /* JERRY_ENGINE_OBJECTCONTEXT_H_ */
