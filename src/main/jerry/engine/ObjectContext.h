/*
MIT License
Copyright (c) 2019-2021 Sven Lukas

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:
The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
*/

#ifndef JERRY_ENGINE_OBJECTCONTEXT_H_
#define JERRY_ENGINE_OBJECTCONTEXT_H_

#include <esl/object/Interface.h>
//#include <esl/object/InitializeContext.h>

#include <string>
#include <map>
#include <vector>
#include <memory>
#include <functional>

namespace jerry {
namespace engine {

//class ObjectContext : virtual public esl::object::Interface::ObjectContext, public esl::object::InitializeContext {
class ObjectContext : public esl::object::Interface::ObjectContext {
public:
	void setParent(esl::object::Interface::ObjectContext* objectContext);

	esl::object::Interface::Object& addObject(const std::string& id, const std::string& implementation, const esl::object::Interface::Settings& settings);
	void addObject(const std::string& id, std::unique_ptr<esl::object::Interface::Object> object) override;
	void addReference(const std::string& id, const std::string& refId);


	//void initializeContext(esl::object::Interface::ObjectContext& objectContext) override;
	virtual void initializeContext();
	virtual void dumpTree(std::size_t depth) const;
	esl::object::Interface::Object* getObject() const;

protected:
	esl::object::Interface::Object* findRawObject(const std::string& id) const override;
	const std::map<std::string, std::reference_wrapper<esl::object::Interface::Object>>& getObjects() const;

private:
	esl::object::Interface::ObjectContext* parent = nullptr;
	std::map<std::string, std::unique_ptr<esl::object::Interface::Object>> objects;
	std::map<std::string, std::reference_wrapper<esl::object::Interface::Object>> objectRefsById;

	void addReference(const std::string& id, esl::object::Interface::Object& object);
};

} /* namespace engine */
} /* namespace jerry */

#endif /* JERRY_ENGINE_OBJECTCONTEXT_H_ */
