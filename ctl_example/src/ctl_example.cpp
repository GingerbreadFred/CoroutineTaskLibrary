#include <ctl/task_queue.h>
#include <ctl/task.h>

#include <array>
#include <iostream>
#include <thread>

// -----------------------------------------------------------------------------
using namespace std::chrono_literals;

ctl::task_handle print_number(const int number)
{
    std::cout << number << '\n';

    co_return;
}

// -----------------------------------------------------------------------------

ctl::task_handle test(ctl::task_queue& taskQueue)
{
    std::array< ctl::task_function, 10 > functions;

    for (int i = 0; i < 10; ++i)
    {
        functions[i] = [=]() { return print_number(i); };
    }

    auto counter = taskQueue.push_waitable_tasks(functions);

    std::cout << "waiting\n";

    co_await ctl::suspend_until(counter);

    std::cout << "done\n";
}

// -----------------------------------------------------------------------------

ctl::const_task_counter_ptr tick_system(const std::string& name, const int number, ctl::task_queue& taskQueue)
{
    std::vector<ctl::task_function> functions;

    for (int i = 0; i < number; ++i)
    {
        auto func = [=]() -> ctl::task_handle
        {
            std::this_thread::sleep_for(std::chrono::milliseconds(rand() % 500));
            std::ostringstream ss; ss << std::this_thread::get_id() << "\t" << name << " " << i << '\n';
            std::cout << ss.str(); 
            co_return;
        };

        functions.push_back(func);
    }

    return taskQueue.push_waitable_tasks(functions);
}

// -----------------------------------------------------------------------------

ctl::task_handle game_loop(ctl::task_queue& taskQueue)
{
    ctl::const_task_counter_ptr animation = tick_system("Animation", 10, taskQueue);
    ctl::const_task_counter_ptr audio = tick_system("Audio", 10, taskQueue);

    co_await ctl::suspend_until(audio);
    co_await ctl::suspend_until(animation);

    ctl::const_task_counter_ptr physics = tick_system("Physics", 10, taskQueue);

    co_await ctl::suspend_until(physics);

    std::cout << "done\n";
}

// -----------------------------------------------------------------------------

int main()
{
    ctl::task_queue queue;

    std::thread thread1([&]() { queue.initialize_thread(); });
    std::thread thread2([&]() { queue.initialize_thread(); });
    
    while (true)
    {
        ctl::const_task_counter_ptr ptr = queue.push_waitable_task([&]() { return game_loop(queue); });
        ptr->wait(1);
        std::this_thread::sleep_for(1000ms);
    }
}

// -----------------------------------------------------------------------------
// -----------------------------------------------------------------------------
// -----------------------------------------------------------------------------
