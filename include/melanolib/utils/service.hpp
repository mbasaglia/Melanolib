/**
 * \file
 *
 * \author Mattia Basaglia
 *
 * \copyright Copyright (C) 2016 Mattia Basaglia
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */
#ifndef MELANOLIB_UTILS_SERVICE_HPP
#define MELANOLIB_UTILS_SERVICE_HPP

#include <atomic>
#include <thread>
#include <condition_variable>
#include <mutex>

namespace melanolib {

template<class Class, class Return=void, class... Arg>
    class MethodCaller
    {
    public:
        using FunctionPtr = Return (Class::*)(Arg...);

        MethodCaller(Class* object, FunctionPtr function)
            : object(object), function(function)
        {}

        template<class... Args>
            Return operator()(Args&&... args) const
        {
            return (object->*function)(std::forward<Args>(args)...);
        }

    private:
        Class* object;
        FunctionPtr function;
    };

template<class Class, class Return=void>
    using ServiceMethodCaller = MethodCaller<Class, Return, std::unique_lock<std::mutex>&>;

/**
 * \brief Utility class for services/daemons
 */
template<class OnStart, class OnStop, class Loop>
class Service
{
public:
    Service(
        OnStart on_start,
        OnStop on_stop,
        Loop loop
    ) : should_run(false),
        on_start(std::move(on_start)),
        on_stop(std::move(on_stop)),
        loop(std::move(loop))
    {}

    Service(const Service&) = delete;
    Service& operator=(const Service&) = delete;

    ~Service()
    {
        stop();
    }

    /**
     * \brief Starts the service in a background thread.
     * \return \b true on success.
     * It will fail if the service is already running
     */
    bool start()
    {
        if ( !running() )
        {
            auto guard = lock();
            if ( !running() )
            {
                should_run = true;
                on_start(guard);
                if ( !guard.owns_lock() )
                    guard.lock();
                thread = std::thread([this]{
                    auto guard = lock();
                    loop(guard);
                });
                return true;
            }
        }
        return false;
    }

    /**
     * \brief Stops the running instance (if running)
     */
    void stop()
    {
        if ( running() )
        {
            auto guard = lock();
            should_run = false;
            on_stop(guard);
            guard.unlock();
            condition_variable.notify_all();
            if ( thread.joinable() )
            {
                thread.join();
            }
        }
    }

    /**
     * \brief Runs the service in the current thread synchronously
     * \returns \b true on success.
     * It will fail if the service is already running
     */
    bool run()
    {
        if ( !running() )
        {
            auto guard = lock();
            if ( !running() )
            {
                should_run = true;
                on_start(guard);
                if ( !guard.owns_lock() )
                    guard.lock();
                loop(guard);
                return true;
            }
        }
        return false;
    }

    /**
     * \brief Whether the service is currently running
     */
    bool running() const
    {
        return thread.joinable() || should_run;
    }

    /**
     * \brief Returns a lock to the internal mutex
     */
    std::unique_lock<std::mutex> lock()
    {
        return std::unique_lock<std::mutex>{mutex};
    }

    std::condition_variable& condition()
    {
        return condition_variable;
    }

protected:
    std::atomic<bool> should_run;

private:
    std::thread thread;
    std::condition_variable condition_variable;
    std::mutex mutex;
    OnStart on_start;
    OnStop on_stop;
    Loop loop;
};

/**
 * \brief Utility class for services/daemons that use a condition variable and loop
 */
template<class OnStart, class OnStop, class Action, class WaitCondition>
class LoopService : public Service<OnStart, OnStop,
    ServiceMethodCaller<LoopService<OnStart, OnStop, Action, WaitCondition>>>
{
public:
    LoopService(
        OnStart on_start,
        OnStop on_stop,
        Action action,
        WaitCondition wait_condition
    ) :  Service<OnStart, OnStop,
    ServiceMethodCaller<LoopService<OnStart, OnStop, Action, WaitCondition>>>(std::move(on_start), std::move(on_stop), {this, &LoopService::loop}),
        action(std::move(action)),
        wait_condition(std::move(wait_condition))
    {}

private:
    /**
     * \brief Main loop for the service.
     *
     * The defaul implementation locks the condition variable while
     * wait_condition() returns true, then it calls action() and stops
     * after stop() is called
     */
    void loop(std::unique_lock<std::mutex>& lock)
    {
        while( this->should_run )
        {
            while ( wait_condition() )
            {
                this->condition().wait(lock);
//                 condition.wait_for(lock, std::chrono::seconds(5));

                if ( !this->should_run )
                    return;
            }

            action(lock);
        }
    }

    /**
     * \brief Action to perform at every loop() iteration
     */
    Action action;

    /**
     * \brief Condition for the condition_variable
     */
    WaitCondition wait_condition;
};

template<class Class>
using CallerService = Service<ServiceMethodCaller<Class>, ServiceMethodCaller<Class>, ServiceMethodCaller<Class>>;

} // namespace melanolib
#endif // MELANOLIB_UTILS_SERVICE_HPP
