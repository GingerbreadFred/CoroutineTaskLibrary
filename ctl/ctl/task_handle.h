#pragma once

#include <coroutine>
#include <ctl/task_counter.h>

// -----------------------------------------------------------------------------

namespace ctl
{
    struct task_handle
    {
        struct promise_type
        {
            task_handle get_return_object()
            {
                return task_handle(std::coroutine_handle<promise_type>::from_promise(*this));
            }

            auto initial_suspend() noexcept { return std::suspend_never(); }
            auto final_suspend() noexcept { return std::suspend_always(); }

            void return_void() {}

            void set_counter(const_task_counter_ptr counter)
            {
                m_counter = std::move(counter);
            }

            const_task_counter_ptr get_counter() const
            {
                return m_counter;
            }

            bool waiting() const
            {
                return m_counter && *m_counter > 0;
            }

            void unhandled_exception() {}


        private:

            const_task_counter_ptr m_counter;
        };


        // -----------------------------------------------------------------------------

        explicit task_handle(std::coroutine_handle<promise_type> coroutine)
            : m_coroutine(coroutine)
        {
        }

        // -----------------------------------------------------------------------------

        task_handle() = default;
        task_handle(task_handle&& other)
            : m_coroutine(std::move(other.m_coroutine))
        {
            other.m_coroutine = nullptr;
        }

        // -----------------------------------------------------------------------------

        task_handle& operator=(task_handle&& other)
        {
            m_coroutine = std::move(other.m_coroutine);
            other.m_coroutine = nullptr;

            return *this;
        }

        // -----------------------------------------------------------------------------

        void resume()
        {
            m_coroutine.resume();
        }

        // -----------------------------------------------------------------------------

        bool complete() const
        {
            return m_coroutine.done();
        }

        // -----------------------------------------------------------------------------

        bool valid() const
        {
            return !!m_coroutine;
        }

        // -----------------------------------------------------------------------------

        bool waiting() const
        {
            return m_coroutine.promise().waiting();
        }

        // -----------------------------------------------------------------------------

        void destroy()
        {
            m_coroutine.destroy();
            m_coroutine = nullptr;
        }

        // -----------------------------------------------------------------------------

        const_task_counter_ptr get_counter() const
        {
            return m_coroutine.promise().get_counter();
        }

        // -----------------------------------------------------------------------------

        task_handle(const task_handle&) = delete;
        task_handle& operator=(const task_handle&) = delete;

    private:

        std::coroutine_handle<promise_type> m_coroutine;
    };
}

// -----------------------------------------------------------------------------
// -----------------------------------------------------------------------------
// -----------------------------------------------------------------------------
