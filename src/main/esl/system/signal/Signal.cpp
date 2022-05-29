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

#include <esl/system/signal/Signal.h>
#include <esl/system/SignalHandler.h>
#include <esl/object/Value.h>

#include <condition_variable>
#include <thread>

namespace esl {
namespace system {
namespace signal {

namespace {
std::mutex instanceMutex;
std::unique_ptr<Signal> instance;

std::mutex newMessagCVMutex;
std::condition_variable newMessagCV;
} /* anonymous namespace */

class Handler : public object::Interface::Object {
public:
	Handler(Interface::SignalType aSignalType, object::Event& aEvent)
	: signalType(aSignalType),
	  event(aEvent)
	{ }

	~Handler() {
		if(Signal::uninstallEventHandler(signalType, event)) {
			std::unique_lock<std::mutex> lockNewMessagCVMutex(newMessagCVMutex);
			newMessagCV.wait(lockNewMessagCVMutex, [] {
					return !instance;
			});
		}
	}

private:
	Interface::SignalType signalType;
	object::Event& event;
};



std::unique_ptr<object::Interface::Object> Signal::install(object::Event& event, Interface::SignalType signalType) {
	std::lock_guard<std::mutex> lockInstanceMutext(instanceMutex);
	if(!instance) {
		std::unique_lock<std::mutex> lockNewMessagCVMutex(newMessagCVMutex);

		instance.reset(new Signal);
		std::thread(Signal::run).detach();

		newMessagCV.wait(lockNewMessagCVMutex);
	}

	std::lock_guard<std::mutex> lockRegistryMutex(instance->registryMutex);
	auto registryInsertResult = instance->registry.insert({signalType, {}});
	if(registryInsertResult.second == true) {
		instance->installSignalHandler(signalType);
	}

	std::set<object::Event*>& events = registryInsertResult.first->second;
	auto eventsInsertResult = events.insert(&event);
	if(eventsInsertResult.second == false) {
		return nullptr;
	}

	return std::unique_ptr<object::Interface::Object>(new Handler(signalType, event));
}


bool Signal::uninstallEventHandler(Interface::SignalType signalType, object::Event& event) {
	std::lock_guard<std::mutex> lockInstanceMutext(instanceMutex);
	if(!instance) {
		return false;
	}

	std::lock_guard<std::mutex> lockRegistryMutex(instance->registryMutex);

	auto registryFindResult = instance->registry.find(signalType);
	if(registryFindResult != instance->registry.end()) {
		std::set<object::Event*>& events = registryFindResult->second;
		events.erase(&event);

		if(events.empty()) {
			instance->registry.erase(registryFindResult);
			instance->uninstallSignalHandler(signalType);
		}
	}

	if(instance->registry.empty()) {
		{
			std::lock_guard<std::mutex> lockMessagesMutex(instance->messagesMutex);
			instance->messages.push_back(std::unique_ptr<Interface::SignalType>(nullptr));
		}

		newMessagCV.notify_one();
		return true;
	}
	return false;
}

void Signal::installSignalHandler(Interface::SignalType signalType) {
	SignalHandler::install(signalType, [signalType]{signalHandler(signalType);});
}

void Signal::uninstallSignalHandler(Interface::SignalType signalType) {
	SignalHandler::remove(signalType, [signalType]{signalHandler(signalType);});
}

void Signal::signalHandler(Interface::SignalType signalType) {
	std::lock_guard<std::mutex> lockInstacneMutext(instanceMutex);

	std::lock_guard<std::mutex> lockMessagesMutex(instance->messagesMutex);
	instance->messages.push_back(std::unique_ptr<Interface::SignalType>(new Interface::SignalType(signalType)));

	newMessagCV.notify_one();
}

void Signal::run() {
	newMessagCV.notify_one();

	std::unique_lock<std::mutex> lockNewMessagCVMutex(newMessagCVMutex);
	while(true) {
		newMessagCV.wait(lockNewMessagCVMutex, [] {
				std::lock_guard<std::mutex> lockMessagesMutex(instance->messagesMutex);
				return !instance->messages.empty();
		});

		std::unique_ptr<Interface::SignalType> message;
		{
			std::lock_guard<std::mutex> lockMessagesMutex(instance->messagesMutex);
			message = std::move(instance->messages.front());
			instance->messages.pop_front();
		}

		if(message) {
			std::lock_guard<std::mutex> lockRegistryMutex(instance->registryMutex);
			esl::object::Value<Interface::SignalType> signalTypeObject(*message);

			auto events = instance->registry.find(*message);
			if(events != instance->registry.end()) {
				for(auto& event : events->second) {
					try {
						event->onEvent(signalTypeObject);
					}
					catch(...) {
					}
				}
			}
		}
		else {
			bool isEmpty = false;
			{
				std::lock_guard<std::mutex> lockRegistryMutex(instance->registryMutex);
				isEmpty = instance->registry.empty();
			}
			if(isEmpty) {
				instance.reset();
				lockNewMessagCVMutex.unlock();
				newMessagCV.notify_one();
				return;
			}
		}
	}
}

} /* namespace signal */
} /* namespace system */
} /* namespace esl */
