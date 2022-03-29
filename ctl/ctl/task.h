#pragma once

#include <functional>

#include <ctl/task_counter.h>
#include <ctl/task_handle.h>

// -----------------------------------------------------------------------------

namespace ctl
{

    using task_function = std::function<task_handle(void)>;

    // -----------------------------------------------------------------------------

    namespace impl
    {

        class task
        {
        public:

            task()
                : m_task()
                , m_handle()
                , m_counter()
            {
            }

            // -----------------------------------------------------------------------------

            ~task()
            {
                if (m_handle.valid())
                {
                    m_handle.destroy();
                }
            }

            // -----------------------------------------------------------------------------

            task(task_function func, task_counter_ptr counter)
                : m_handle()
                , m_task(std::move(func))
                , m_counter(std::move(counter))
            {
            }


            task(const task&) = delete;
            task(task&& other)
                : m_handle(std::move(other.m_handle))
                , m_task(std::move(other.m_task))
                , m_counter(std::move(other.m_counter))
            {
                other.m_handle = task_handle();
            }

            task& operator=(const task&) = delete;
            task& operator=(task&& other)
            {
                m_handle = std::move(other.m_handle);
                m_task = std::move(other.m_task);
                m_counter = std::move(other.m_counter);

                other.m_handle = task_handle();

                return *this;
            }

            // -----------------------------------------------------------------------------

            bool is_valid() const
            {
                return static_cast<bool>(m_task);
            }

            // -----------------------------------------------------------------------------

            bool complete() const
            {
                return m_handle.valid() && m_handle.complete();
            }

            // -----------------------------------------------------------------------------

            bool waiting() const
            {
                return m_handle.valid() && m_handle.waiting();
            }

            // -----------------------------------------------------------------------------

            const_task_counter_ptr get_waiting_counter() const
            {
                return m_handle.get_counter();
            }

            // -----------------------------------------------------------------------------

            void run()
            {
                if (m_handle.valid() && !m_handle.complete())
                {
                    m_handle.resume();
                }
                else
                {
                    m_handle = m_task();
                }
            }

            // -----------------------------------------------------------------------------

            task_counter_ptr get_counter() const
            {
                return m_counter;
            }

            // -----------------------------------------------------------------------------


        private:

            task_handle m_handle;
            task_function m_task;
            task_counter_ptr m_counter;
        };
    }
}

// -----------------------------------------------------------------------------
// -----------------------------------------------------------------------------
// -----------------------------------------------------------------------------

