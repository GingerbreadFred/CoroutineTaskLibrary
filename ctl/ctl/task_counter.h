#pragma once

#include <atomic>

// -----------------------------------------------------------------------------

namespace ctl
{
    using task_counter = std::atomic<size_t>;
    using task_counter_ptr = std::shared_ptr<task_counter>;
    using const_task_counter_ptr = std::shared_ptr<const task_counter>;
}

// -----------------------------------------------------------------------------
// -----------------------------------------------------------------------------
// -----------------------------------------------------------------------------
