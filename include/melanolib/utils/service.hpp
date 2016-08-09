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

/**
 * \brief Base class for services/daemons
 */
class ServiceBase
{
public:
    ServiceBase() : should_run(false) {}

    /**
     * \note Derived class destructors should call stop()
     * It isn't called here because run() calls virtual functions
     */
    virtual ~ServiceBase() {}

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
            if ( !guard.owns_lock() )
                guard.lock();
            condition.notify_all();
            guard.unlock();
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

protected:
    /**
     * \brief Main loop for the service.
     *
     * The defaul implementation locks the condition variable while
     * wait_condition() returns true, then it calls action() and stops
     * after stop() is called
     */
    virtual void loop(std::unique_lock<std::mutex>& lock) = 0;
    virtual void on_start(std::unique_lock<std::mutex>& lock) {}
    virtual void on_stop(std::unique_lock<std::mutex>& lock) {}

    /**
     * \brief Returns a lock to the internal mutex
     */
    std::unique_lock<std::mutex> lock()
    {
        return std::unique_lock<std::mutex>{mutex};
    }

    std::thread thread;
    std::condition_variable condition;
    std::mutex mutex;
    std::atomic<bool> should_run;
};

/**
 * \brief Base class for services/daemons that use a condition variable and loop
 */
class Service : public ServiceBase
{
protected:
    /**
     * \brief Main loop for the service.
     *
     * The defaul implementation locks the condition variable while
     * wait_condition() returns true, then it calls action() and stops
     * after stop() is called
     */
    virtual void loop(std::unique_lock<std::mutex>& lock)
    {
        while( should_run )
        {
            while ( wait_condition() && should_run )
            {
                condition.wait(lock);
            }

            if ( !should_run )
                return;

            action(lock);
        }
    }

    /**
     * \brief Condition for the condition_variable
     */
    virtual bool wait_condition()
    {
        return false;
    }

    /**
     * \brief Action to perform at every loop() iteration
     */
    virtual void action(std::unique_lock<std::mutex>& lock) = 0;

    std::condition_variable condition;
};

} // namespace melanolib
#endif // MELANOLIB_UTILS_SERVICE_HPP
