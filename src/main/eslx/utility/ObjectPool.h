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

#ifndef ESLX_UTILITY_OBJECTPOOL_H_
#define ESLX_UTILITY_OBJECTPOOL_H_

#include <esl/logging/Logger.h>
#include <chrono>
#include <list>
#include <memory>
#include <mutex>
#include <condition_variable>
#include <thread>

namespace eslx {
namespace utility {

template<class Object>
class ObjectPool {
	static esl::logging::Logger<> logger;

private:
	struct Deleter {
		static esl::logging::Logger<> logger;

		Deleter(ObjectPool& aObjectPool, std::chrono::steady_clock::time_point aTimePointBegin)
		: objectPool(&aObjectPool),
		  timePointBegin(aTimePointBegin)
		{ }
		Deleter() = default;

		void operator()(Object* object) const {
			if(object == nullptr) {
				ESL__LOGGER_ERROR_THIS("object == nullptr\n");
				return;
			}

			std::unique_ptr<Object> objectPtr(object);
			ESL__LOGGER_DEBUG_THIS("object == ", object, "\n");

			if(objectPool) {
				objectPool->release(std::move(objectPtr), timePointBegin);
			}
			else {
				ESL__LOGGER_DEBUG_THIS("objectPool has been detached\n");
			}
		}

		ObjectPool* objectPool = nullptr;
		mutable std::chrono::steady_clock::time_point timePointBegin;
	};

public:
	enum class Strategy {
		lifo, fifo
	};
	using CreateObject = std::function<std::unique_ptr<Object>()>;
	using unique_ptr = std::unique_ptr<Object, Deleter>;

	/**
	 * Constructor
	 *
	 * @param[in] createObject Function object without arguments that returns unique_ptr<Object> to create an object for the pool
	 * @param[in] objectsMax Maximum amount of objects that the pool does allow.
	 *            A value of 0 means infinity number of objects.
	 * @param[in] objectLifetime Maximum lifetime of an object in this pool.
	 *                           A value of 0 means infinity lifetime.
	 * @param[in] resetLifetimeOnGet Specifies how lifetime of objects are handled if calling get(...).
	 *                               If this value is set to false the objects lifetime will not be touched when taking it out of the pool.
	 *                               If this value is set to true the objects lifetime will be reseted to its original lifetime when taking it out of the pool.
	 * @param[in] resetLifetimeOnRelease Specifies how lifetime of objects are handled when they are released back to the pool.
	 *                                   If this value is set to false the objects lifetime will not be touched when taking it back to the pool.
	 *                                   If this value is set to true the objects lifetime will be reseted to its original lifetime when taking it back to the pool.
	 */
	ObjectPool(CreateObject createObject, size_t objectsMax, std::chrono::nanoseconds objectLifetime, bool resetLifetimeOnGet, bool resetLifetimeOnRelease);
	~ObjectPool();

	/**
	 * Returns a object from object pool. If maximum number of objects are circulating already outside this pool, then this function will wait.
	 * If a timeout>0 is specified, it will return not later than this time but it will return an empty object (nullptr) if there is still no
	 * more space free for getting an object.
	 * If ObjectPool gets destroyed this function will also return immediately with an empty object (nullptr).
	 *
	 * @param[in] timeout Duration to wait for getting an object. A value of 0 means no wait.
	 * @param[in] strategy Specifies which object is returned form the pool, if no new object has to be created.
	 *                     If strategy is set to Strategy::lifo, then the object that has been latest released back to the pool is returned.
	 *                     If stratefy is set to Strategy::lifo, then the object will be returned that is remaining longest time in the pool.
	 */
	unique_ptr get(std::chrono::nanoseconds timeout, Strategy strategy = Strategy::fifo);
	unique_ptr get(Strategy strategy = Strategy::fifo);
	//std::unique_ptr<Object> getObject();

private:
    void timeoutHandler();
	void release(std::unique_ptr<Object> object, std::chrono::steady_clock::time_point timePointBegin/*, bool destroy*/);
	bool checkGetObject() const;
	bool isTimeoutOrDirty(const Object& object, std::chrono::steady_clock::time_point timePointBegin, std::chrono::steady_clock::time_point timePointNow) const;
    static bool isDirty(const Object& object);

	const CreateObject createObject;
	const size_t objectsMax;

	/* objectLifetime==0 means infinity lifetime.
	 * A real lifetime of zero would make no sense, because
	 * the object would be invalid directly after creating it.
	 */
	const std::chrono::nanoseconds objectLifetime;
	const bool resetLifetimeOnGet;
	const bool resetLifetimeOnRelease;

    mutable std::mutex objectsMutex;
    std::condition_variable objectsCv;
    std::condition_variable objectsTimeoutHandlerCv;
    std::list<unique_ptr> objects;
    /* number of objects that are retrieved by calling get()
     * and not put back to the pool so far.
     */
	std::size_t objectsCirculating{0};
    bool descructorCalled{false};

    std::thread timeoutHandlerThread;
};

template<class Object>
esl::logging::Logger<> ObjectPool<Object>::logger("esl::utility::ObjectPool<>");

template<class Object>
esl::logging::Logger<> ObjectPool<Object>::Deleter::logger("esl::utility::ObjectPool<>::Deleter");

template<class Object>
ObjectPool<Object>::ObjectPool(CreateObject aCreateObject, size_t aObjectsMax, std::chrono::nanoseconds aObjectLifetime, bool aResetLifetimeOnGet, bool aResetLifetimeOnRelease)
: createObject(aCreateObject),
  objectsMax(aObjectsMax),
  objectLifetime(aObjectLifetime),
  resetLifetimeOnGet(aResetLifetimeOnGet),
  resetLifetimeOnRelease(aResetLifetimeOnRelease)
{
	/* if objectLifetime is zero, then objects have infinity lifetime and we don't need a thread to delete expired objects */
	if(objectLifetime != std::chrono::nanoseconds(0)) {
		  timeoutHandlerThread = std::thread(&ObjectPool::timeoutHandler, this);
	}
}

template<class Object>
ObjectPool<Object>::~ObjectPool() {
	{
    	std::lock_guard<std::mutex> objectsMutexLock(objectsMutex);
    	descructorCalled = true;
	}

	if(objectLifetime != std::chrono::nanoseconds(0)) {
		/* Notify timeout handler to delete all objects in the pool.
		 * Timeout handler will call "objectsCv.notify_all();" then
		 * to let waiting get-Calls returning an empty object.
		 * Timeout handler will still wait till all circling objects
		 * outside this pool are released. */
		objectsTimeoutHandlerCv.notify_one();

		/* let's wait till timeout handler is done */
		ESL__LOGGER_TRACE_THIS("before \"timeoutHandlerThread.join()\"\n");
		timeoutHandlerThread.join();
		ESL__LOGGER_TRACE_THIS("after \"timeoutHandlerThread.join()\"\n");
	}
	else {
		/* There is no thread running.
		 * So we notify all waiting get-Calls directly to return a nullptr.
		 */
		objectsCv.notify_all();
	}
}

template<class Object>
typename ObjectPool<Object>::unique_ptr ObjectPool<Object>::get(Strategy strategy) {
	return get(std::chrono::nanoseconds(0), strategy);
}

template<class Object>
typename ObjectPool<Object>::unique_ptr ObjectPool<Object>::get(std::chrono::nanoseconds timeout, Strategy strategy) {
	ESL__LOGGER_TRACE_THIS("before lock\n");
	std::unique_lock<std::mutex> objectsMutexLock(objectsMutex);
	ESL__LOGGER_TRACE_THIS("after lock\n");

	// objectsMax == 0 means infinity number of objects, so we don't need to wait
	if(objectsMax > 0) {
    	if(timeout == std::chrono::nanoseconds(0)) {
    		objectsCv.wait(objectsMutexLock, std::bind(&ObjectPool::checkGetObject, this));
    	}
    	else if(!objectsCv.wait_for(objectsMutexLock, timeout, std::bind(&ObjectPool::checkGetObject, this))) {
    		ESL__LOGGER_DEBUG_THIS("timeout\n");
			return unique_ptr(nullptr, Deleter(*this, std::chrono::steady_clock::time_point{}));
    	}
	}

	if(descructorCalled) {
		ESL__LOGGER_TRACE_THIS("descructor called, return nullptr\n");
		return unique_ptr(nullptr, Deleter(*this, std::chrono::steady_clock::time_point{}));
	}

	ESL__LOGGER_TRACE_THIS("++objectsCirculating\n");
    ++objectsCirculating;

	if(objects.empty()) {
		ESL__LOGGER_TRACE_THIS("createObject()\n");
        // create new object
    	std::unique_ptr<Object> object = createObject();
		return unique_ptr(object.release(), Deleter(*this, std::chrono::steady_clock::now()));
	}


	unique_ptr object;
	if(strategy == Strategy::lifo) {
		ESL__LOGGER_TRACE_THIS("LIFO\n");
		object = std::move(objects.back());
		objects.pop_back();
	}
	else {
	// else if(strategy == Strategy::fifo) {
		ESL__LOGGER_TRACE_THIS("FIFO\n");
		object = std::move(objects.front());
		objects.pop_front();
	}

    // use existing object
	if(resetLifetimeOnGet) {
		ESL__LOGGER_TRACE_THIS("resetLifetimeOnGet\n");
		object.get_deleter().timePointBegin = std::chrono::steady_clock::now();
	}

	return object;
}

template<class Object>
void ObjectPool<Object>::timeoutHandler() {
	std::unique_lock<std::mutex> objectsMutexLock(objectsMutex);
	std::chrono::steady_clock::time_point timePointNow = std::chrono::steady_clock::now();

    while(descructorCalled == false || objectsCirculating > 0) {
		ESL__LOGGER_TRACE_THIS("calculate timeoutHandlerWakeup\n");
		std::chrono::nanoseconds timeoutHandlerWakeup{0};
		for(const auto& object : objects) {
    		if(object.get_deleter().timePointBegin + objectLifetime <= timePointNow) {
    			continue;
    		}
			std::chrono::nanoseconds remainingLifetime;
			remainingLifetime = std::chrono::duration_cast<std::chrono::nanoseconds>(object.get_deleter().timePointBegin + objectLifetime - timePointNow);
			if(timeoutHandlerWakeup > remainingLifetime || timeoutHandlerWakeup == std::chrono::nanoseconds(0)) {
				timeoutHandlerWakeup = remainingLifetime;
			}
		}
		ESL__LOGGER_DEBUG_THIS("timeoutHandlerWakeup = ", timeoutHandlerWakeup.count(), "ns\n");

    	if(timeoutHandlerWakeup == std::chrono::nanoseconds(0)) {
    		ESL__LOGGER_TRACE_THIS("timeoutHandlerCv.wait()\n");
    		objectsTimeoutHandlerCv.wait(objectsMutexLock);
    	}
    	else {
    		ESL__LOGGER_TRACE_THIS("timeoutHandlerCv.wait_for(", timeoutHandlerWakeup.count(), "ns)\n");
    		objectsTimeoutHandlerCv.wait_for(objectsMutexLock, timeoutHandlerWakeup);
    	}

		ESL__LOGGER_TRACE_THIS("delete elapsed objects\n");
    	timePointNow = std::chrono::steady_clock::now();
        for(auto iter = objects.begin(); iter != objects.end();) {
        	if(descructorCalled || isTimeoutOrDirty(*iter->get(), iter->get_deleter().timePointBegin, timePointNow)) {
        		// prevent Deleter to call "ObjectPool::release"
        		iter->get_deleter().objectPool = nullptr;

        		ESL__LOGGER_DEBUG_THIS("erase object = ", iter->get(), "\n");
        		iter = objects.erase(iter);
        	}
        	else {
        		++iter;
        	}
        }
        objectsCv.notify_all();
    }
	ESL__LOGGER_TRACE_THIS("RETURN\n");
}

template<class Object>
void ObjectPool<Object>::release(std::unique_ptr<Object> object, std::chrono::steady_clock::time_point timePointBegin) {
	if(!object) {
		ESL__LOGGER_ERROR_THIS("called with nullptr object\n");
		return;
	}

	ESL__LOGGER_TRACE_THIS("before lock\n");
	std::lock_guard<std::mutex> objectsMutexLock(objectsMutex);
	ESL__LOGGER_TRACE_THIS("after lock\n");

	if(descructorCalled) {
    	ESL__LOGGER_DEBUG_THIS("Destroy object because ~ObjectPool() has been called.\n");
	}
	else {
		std::chrono::steady_clock::time_point timePointNow = std::chrono::steady_clock::now();

		if(isTimeoutOrDirty(*object.get(), timePointBegin, timePointNow)) {
	    	ESL__LOGGER_DEBUG_THIS("Destroy object because timeout occurred or object is dirty.\n");
	    }
	    else {
    	    // renew Object
    		if(resetLifetimeOnRelease) {
    			ESL__LOGGER_TRACE_THIS("resetLifetimeOnRelease\n");
    			timePointBegin = timePointNow;
    		}

	    	ESL__LOGGER_DEBUG_THIS("objects.emplace_back(...) with object=", object.get(), "\n");
    		objects.emplace_back(unique_ptr(object.release(), Deleter(*this, timePointBegin)));
        }
	}

	--objectsCirculating;
    /* notify to let's get a new object or to if in exit-mode to return nullptr */
    objectsCv.notify_one();

	if(objectLifetime != std::chrono::nanoseconds(0)) {
	    /* notify to calulate new wait time or if in exit-mode to check if objectsCirculating == 0 */
		objectsTimeoutHandlerCv.notify_one();
	}
}

template<class Object>
bool ObjectPool<Object>::checkGetObject() const {
	return descructorCalled || objectsMax == 0 || objectsMax > objectsCirculating;
}

template<class Object>
bool ObjectPool<Object>::isTimeoutOrDirty(const Object& object, std::chrono::steady_clock::time_point timePointBegin, std::chrono::steady_clock::time_point timePointNow) const {
    return (objectLifetime > std::chrono::nanoseconds(0) && timePointBegin + objectLifetime <= timePointNow) || isDirty(object);
}

template<class Object>
bool ObjectPool<Object>::isDirty(const Object& object) {
	return false;
}

} /* namespace utility */
} /* namespace eslx */

#endif /* ESLX_UTILITY_OBJECTPOOL_H_ */
