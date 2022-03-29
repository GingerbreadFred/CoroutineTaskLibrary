#include <CppUnitTest.h>
#include <ctl/task_handle.h>

// -----------------------------------------------------------------------------

using namespace Microsoft::VisualStudio::CppUnitTestFramework;

// -----------------------------------------------------------------------------

namespace ctl_test
{
    TEST_CLASS(task_handle)
    {
    public:

        ctl::task_handle test_suspend_always(const int loopCount)
        {
            for (int i = 0; i < loopCount; ++i)
            {
                co_await std::suspend_always{};
            }
        }

        // -----------------------------------------------------------------------------

        TEST_METHOD(constructed_and_incomplete_complete_returns_false)
        {
            ctl::task_handle handle = test_suspend_always(1);

            Assert::IsFalse(handle.complete());
        }

        // -----------------------------------------------------------------------------

        TEST_METHOD(constructed_and_complete_complete_returns_true)
        {
            ctl::task_handle handle = test_suspend_always(0);

            Assert::IsTrue(handle.complete());
        }

        // -----------------------------------------------------------------------------

        TEST_METHOD(resumed_not_to_completion_complete_returns_false)
        {
            ctl::task_handle handle = test_suspend_always(2);
            handle.resume();

            Assert::IsFalse(handle.complete());
        }

        // -----------------------------------------------------------------------------

        TEST_METHOD(resumed_to_completion_complete_returns_true)
        {
            ctl::task_handle handle = test_suspend_always(2);
            handle.resume();
            handle.resume();

            Assert::IsTrue(handle.complete());
        }

        // -----------------------------------------------------------------------------

        TEST_METHOD(resumed_not_to_completion_moved_from_moved_to_is_valid)
        {
            ctl::task_handle handle = test_suspend_always(2);
            handle.resume();

            ctl::task_handle newHandle = std::move(handle);

            Assert::IsFalse(handle.valid(), L"Moved from handle should be invalid");
            Assert::IsTrue(newHandle.valid(), L"Moved to handle should be valid");
        }

        // -----------------------------------------------------------------------------

        TEST_METHOD(moved_to_and_resumed_to_completion_complete_returns_true)
        {
            ctl::task_handle handle = test_suspend_always(2);
            handle.resume();

            ctl::task_handle newHandle = std::move(handle);

            newHandle.resume();

            Assert::IsTrue(newHandle.complete());
        }
    };
}

// -----------------------------------------------------------------------------
// -----------------------------------------------------------------------------
// -----------------------------------------------------------------------------
