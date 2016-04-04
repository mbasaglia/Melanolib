/**
 * \file
 *
 * \author Mattia Basaglia
 *
 * \copyright Copyright (C) 2015-2016 Mattia Basaglia
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
#ifndef MELANOLIB_TIME_TIMER_HPP
#define MELANOLIB_TIME_TIMER_HPP

#include <chrono>
#include <mutex>
#include <atomic>
#include <condition_variable>
#include <thread>

#include "melanolib/utils/functional.hpp"

namespace melanolib {
namespace time {

/**
 * \brief A timer which performs a task after some time on a separate thread
 * \tparam Clock A clock type
 */
template<class Clock>
    class basic_timer
    {
    public:
        using clock_type    = Clock;                    ///< Clock
        using duration_type = typename Clock::duration; ///< Duration
        using function_type = std::function<void()>;    ///< Callback type

        explicit basic_timer( function_type action = {},
                              duration_type timeout = duration_type::zero(),
                              bool repeating = true
                            )
            :
            timeout(std::move(timeout)),
            repeating(repeating),
            action(std::move(action))
        {}

        basic_timer(const basic_timer& rhs)
            : timeout(rhs.timeout), repeating(rhs.repeating), action(rhs.action)
        {}

        basic_timer& operator=(const basic_timer& rhs)
        {
            timeout = rhs.timeout;
            repeating = rhs.repeating.load();
            action = rhs.action;
            return *this;
        }

        // note: mutex not move constructible
        basic_timer(basic_timer&& rhs) :
            timeout     (std::move(rhs.timeout)),
            repeating   (std::move(rhs.repeating)),
            action      (std::move(rhs.action))
        {
            rhs.stop();
        }

        basic_timer& operator=(basic_timer&& rhs)
        {
            rhs.stop();
            timeout     = std::move(rhs.timeout);
            repeating   = rhs.repeating.load();
            action      = std::move(rhs.action);
            return *this;
        }

        ~basic_timer() { stop(); }

        /**
         * \brief Start the timer with the already-set duration
         * \note If \c timeout is zero, \c repeating will be ignored
         * \return \b true on success
         */
        bool start()
        {
            if ( running() || !action )
                return false;
            if ( timeout <= duration_type::zero() )
                callback(action);
            else
                thread = std::move(std::thread([this]{run();}));
            return running();
        }

        /**
         * \brief Whether the timer has been started and is still running
         */
        bool running() const
        {
            return thread.joinable();
        }

        /**
         * \brief Stop the timer and set a new timeout
         * \return \b true on success
         */
        bool reset(duration_type timeout)
        {
            stop();
            this->timeout = timeout;
            return start();
        }

        /**
         * \brief Stop the timer
         */
        void stop()
        {
            if ( running() )
            {
                active = false;
                condition.notify_all();
                try {
                    thread.join();
                // catch in the case the thread joined early
                } catch(const std::invalid_argument&) {}
            }
        }

    private:
        duration_type           timeout;         ///< Timer duration
        std::atomic<bool>       repeating{false};///< Whether it should start over once finished
        std::atomic<bool>       active{false};   ///< Whether it's currently running
        std::condition_variable condition;       ///< Wait condition
        function_type           action;          ///< Executed on timeout
        std::thread             thread;          ///< Thread where the callback is executed

        /**
         * \brief Runs the timer
         */
        void run()
        {
            std::mutex mutex;
            active = true;
            while ( active && repeating )
            {
                std::unique_lock<std::mutex> lock(mutex);
                condition.wait_until(lock,clock_type::now()+timeout,[this]{return !active;});
                if ( active )
                    action();
            }
            active = false;
        }

    };

/**
 * \brief Timer using the wall clock
 */
using Timer = basic_timer<std::chrono::system_clock>;


} // namespace time
} // namespace melanolib
#endif // MELANOLIB_TIME_TIMER_HPP
