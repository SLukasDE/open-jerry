/*
MIT License
Copyright (c) 2019-2022 Sven Lukas

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

#ifndef ESL_SYSTEM_SIGNAL_SIGNAL_H_
#define ESL_SYSTEM_SIGNAL_SIGNAL_H_

#include <esl/system/Interface.h>
#include <esl/object/Event.h>
#include <esl/object/Interface.h>

#include <list>
#include <map>
#include <memory>
#include <mutex>
#include <set>

namespace esl {
namespace system {
namespace signal {

class Handler;

class Signal final {
public:
	friend class Handler;

	static std::unique_ptr<object::Interface::Object> install(object::Event& event, Interface::SignalType signalType);

private:
	std::mutex registryMutex;
	std::map<Interface::SignalType, std::set<object::Event*>> registry;

	std::mutex messagesMutex;
	std::list<std::unique_ptr<Interface::SignalType>> messages;

	Signal() = default;

	static bool uninstallEventHandler(Interface::SignalType signalType, object::Event& event);
	void installSignalHandler(Interface::SignalType signalType);
	void uninstallSignalHandler(Interface::SignalType signalType);
	static void signalHandler(Interface::SignalType signalType);
	static void run();
};

} /* namespace signal */
} /* namespace system */
} /* namespace esl */

#endif /* ESL_SYSTEM_SIGNAL_SIGNAL_H_ */
