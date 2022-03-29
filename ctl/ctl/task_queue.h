#pragma once

#include <list>
#include <map>
#include <mutex>
#include <queue>
#include <vector>

#include <ctl/task_counter.h>
#include <ctl/task.h>
#include <ctl/sleeping_task.h>

// -----------------------------------------------------------------------------

namespace ctl
{
    namespace impl
    {
        class task;
        class sleeping_task;
    }

    struct suspend_until
    {

        explicit suspend_until(const_task_counter_ptr taskCounter)
            : m_taskCounter(std::move(taskCounter))
        {
        }

        bool await_ready()
        {
            return false;
        }

        void await_suspend(std::coroutine_handle<ctl::task_handle::promise_type> type)
        {
            type.promise().set_counter(m_taskCounter);
        }

        void await_resume()
        {
        }

    private:

        const_task_counter_ptr m_taskCounter;

    };


    // -----------------------------------------------------------------------------

    class task_queue final
    {
    public:

        task_queue()
            : m_mutex()
            , m_queue()
            , m_abort(false)
        {
        }

        ~task_queue() = default;

        // -----------------------------------------------------------------------------

        task_queue(task_queue&&) = default;
        task_queue& operator=(task_queue&&) = default;
        task_queue(const task_queue&) = delete;
        task_queue& operator=(const task_queue&) = delete;

        // -----------------------------------------------------------------------------

        void initialize_thread()
        {
            run_task_thread();
        }

        // -----------------------------------------------------------------------------

        void abort()
        {
            m_abort = true;
            m_waitForTasks.notify_all();
        }

        // -----------------------------------------------------------------------------

        template< typename T >
        task_counter_ptr push_waitable_tasks(const T& taskCollection)
        {
            auto counter = std::make_shared<task_counter>(taskCollection.size());

            push_tasks_impl(taskCollection, counter);

            return counter;
        }

        // -----------------------------------------------------------------------------

        template< typename T >
        void push_tasks(const T& taskCollection)
        {
            push_tasks_impl(taskCollection, nullptr);
        }

        // -----------------------------------------------------------------------------

        template< typename T >
        task_counter_ptr push_waitable_task(T task)
        {
            auto counter = std::make_shared<task_counter>(1u);

            push_task_impl(std::move(task), counter);

            return counter;
        }

        // -----------------------------------------------------------------------------

        template< typename T >
        void push_task(T task)
        {
            push_task_impl(std::move(task), nullptr);
        }

        // -----------------------------------------------------------------------------


    private:

        void run_task_thread()
        {
            while (!m_abort)
            {
                run_next_available_task();
                wait_for_tasks();
            }
        }

        // -----------------------------------------------------------------------------

        void wait_for_tasks()
        {
            std::unique_lock<std::mutex> lk(m_mutex);

            if (m_queue.empty())
            {
                m_waitForTasks.wait(lk);
            }
        }

        // -----------------------------------------------------------------------------

        void run_next_available_task()
        {
            impl::task task;
            {
                std::lock_guard<std::mutex> lock(m_mutex);
                if (!m_queue.empty())
                {
                    task = std::move(m_queue.front());
                    m_queue.erase(m_queue.begin());
                }
            }

            if (task.is_valid())
            {
                task.run();

                if (!task.complete())
                {
                    if (task.waiting())
                    {
                        std::lock_guard<std::mutex> lock(m_mutex);
                        m_sleepingTasks.emplace_back(std::move(task));
                    }
                    else
                    {
                        std::lock_guard<std::mutex> lock(m_mutex);
                        m_queue.push_back(std::move(task));
                    }

                    m_waitForTasks.notify_all();
                }
                else
                {
                    auto counter = task.get_counter();

                    if (counter && counter->fetch_sub(1) == 1)
                    {
                        std::lock_guard< std::mutex > lock(m_mutex);

                        counter->notify_all();

                        for (auto iter = m_sleepingTasks.begin(); iter != m_sleepingTasks.end(); )
                        {
                            if (iter->get_task().get_waiting_counter() == counter)
                            {
                                m_queue.emplace_back(std::move(iter->get_task()));
                                iter = m_sleepingTasks.erase(iter);
                                m_waitForTasks.notify_all();
                            }
                            else
                            {
                                ++iter;
                            }
                        }
                    }
                }
            }
        }

        // -----------------------------------------------------------------------------

        template< typename T >
        void push_tasks_impl(const T& taskCollection, task_counter_ptr counter)
        {
            {
                std::lock_guard<std::mutex> lock(m_mutex);

                for (auto& task : taskCollection)
                {
                    m_queue.emplace_back(task, counter);
                }
            }

            m_waitForTasks.notify_all();
        }

        // -----------------------------------------------------------------------------

        template< typename T >
        void push_task_impl(T task, task_counter_ptr counter)
        {
            {
                std::lock_guard<std::mutex> lock(m_mutex);
                m_queue.emplace_back(std::move(task), std::move(counter));
            }

            m_waitForTasks.notify_all();
        }

        // -----------------------------------------------------------------------------

        std::atomic<bool> m_abort;
        std::mutex m_mutex;
        std::condition_variable m_waitForTasks;

        // -----------------------------------------------------------------------------

        std::vector<ctl::impl::task> m_queue;
        std::vector<ctl::impl::sleeping_task> m_sleepingTasks;
    };
}

// -----------------------------------------------------------------------------
// -----------------------------------------------------------------------------
// -----------------------------------------------------------------------------
