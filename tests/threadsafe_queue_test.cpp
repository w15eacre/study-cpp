#include <algorithm>
#include <chrono>
#include <future>
#include <ranges>
#include <thread>

#include <gtest/gtest.h>

import threadsafe_queue;
import threadsafe_queue_mutex;
import threadsafe_std_queue;

namespace {

template <typename Queue>
class ThreadSafeQueueTypedFixture : public ::testing::Test
{
public:
    ~ThreadSafeQueueTypedFixture() noexcept = default;
    using QueueType = Queue;
};

} // namespace

using QueueTypes = ::testing::
    Types<threadsafe_queue::ThreadSafeQueue<int>, threadsafe_queue::ThreadSafeQueueMutex<int>, threadsafe_queue::ThreadSafeStdQueue<int>>;
TYPED_TEST_SUITE(ThreadSafeQueueTypedFixture, QueueTypes);

TYPED_TEST(ThreadSafeQueueTypedFixture, PushTryPopSingleThreadTest_ExpectTheSameOrder)
{
    typename TestFixture::QueueType queue{};

    auto ExpectedValues = std::views::iota(0, 100);

    for (auto i : ExpectedValues)
    {
        queue.Push(i);
    }

    std::vector<int> actuallyValues{};
    int res = 0;
    while (queue.TryPop(res))
    {
        actuallyValues.push_back(res);
    }

    ASSERT_TRUE(std::ranges::equal(actuallyValues, ExpectedValues));
}

TYPED_TEST(ThreadSafeQueueTypedFixture, PushPopSingleThreadTest_ExpectTheSameOrder)
{
    typename TestFixture::QueueType queue{};

    auto ExpectedValues = std::views::iota(0, 100);

    for (auto i : ExpectedValues)
    {
        queue.Push(i);
    }

    std::vector<int> actuallyValues{};
    ASSERT_THROW(
        while (true) {
            int res = 0;
            queue.Pop(res);
            actuallyValues.push_back(res);
        },
        std::runtime_error);

    ASSERT_TRUE(std::ranges::equal(actuallyValues, ExpectedValues));
}

TYPED_TEST(ThreadSafeQueueTypedFixture, PushPopAndWaitTest_ExpectTheSameOrder)
{
    typename TestFixture::QueueType queue{};

    auto ExpectedValues = std::views::iota(0, 100);

    std::ignore = std::async(std::launch::async, [&queue, ExpectedValues]() {
        for (auto i : ExpectedValues)
        {
            queue.Push(i);
            std::this_thread::sleep_for(std::chrono::milliseconds{10});
        }
    });

    std::vector<int> actuallyValues{};

    while (actuallyValues.size() != ExpectedValues.size())
    {
        int res = 0;
        queue.WaitAndPop(res);
        actuallyValues.push_back(res);
    }

    ASSERT_TRUE(std::ranges::equal(actuallyValues, ExpectedValues));
}

TYPED_TEST(ThreadSafeQueueTypedFixture, PushShutdownTest_ExpectException)
{
    typename TestFixture::QueueType queue{};

    queue.Shutdown();

    ASSERT_THROW(queue.Push(0), std::runtime_error);
}

TYPED_TEST(ThreadSafeQueueTypedFixture, WaitAndPopShutdownTest_ExpectException)
{
    typename TestFixture::QueueType queue{};

    queue.Shutdown();

    int res = 0;
    ASSERT_THROW(queue.WaitAndPop(res), std::runtime_error);
}
