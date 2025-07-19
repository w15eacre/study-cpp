#include <algorithm>
#include <chrono>
#include <future>
#include <ranges>
#include <vector>

#include <gtest/gtest.h>

import threadsafe_queue;

namespace {

class ThreadSafeQueueFixture : public ::testing::Test
{
protected:
    threadsafe_queue::ThreadSafeQueue<int> queue{};
};

struct PerfTestParams
{
    std::string_view name{};
    size_t dataCount{};
    size_t producerCount{};
    size_t consumerCount{};
};

class ThreadSafeQueueFixtureWithParams
    : public ThreadSafeQueueFixture
    , public ::testing::WithParamInterface<PerfTestParams>
{};

template <typename Unit>
concept PerformanceUnit = std::is_same_v<Unit, std::chrono::milliseconds> || std::is_same_v<Unit, std::chrono::microseconds> ||
    std::is_same_v<Unit, std::chrono::nanoseconds> || std::is_same_v<Unit, std::chrono::seconds>;


template <PerformanceUnit Unit, std::invocable Func>
inline void PerformanceTest(std::string_view name, Func&& func) noexcept
{
    auto start = std::chrono::high_resolution_clock::now();
    std::invoke(std::forward<Func>(func));
    auto finish = std::chrono::high_resolution_clock::now();

    std::cout << "[ PERF ] [" << name << "]:  " << std::chrono::duration_cast<Unit>(finish - start) << std::endl;
}


} // namespace

TEST_F(ThreadSafeQueueFixture, PushTryPopSingleThreadTest_ExpectTheSameOrder)
{
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

TEST_F(ThreadSafeQueueFixture, PushPopSingleThreadTest_ExpectTheSameOrder)
{
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

TEST_F(ThreadSafeQueueFixture, PushPopAndWaitTest_ExpectTheSameOrder)
{
    auto ExpectedValues = std::views::iota(0, 100);

    std::ignore = std::async(std::launch::async, [this, ExpectedValues]() {
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

TEST_F(ThreadSafeQueueFixture, PushShutdownTest_ExpectException)
{
    queue.Shutdown();

    ASSERT_THROW(queue.Push(0), std::runtime_error);
}

TEST_F(ThreadSafeQueueFixture, WaitAndPopShutdownTest_ExpectException)
{
    queue.Shutdown();

    int res = 0;
    ASSERT_THROW(queue.WaitAndPop(res), std::runtime_error);
}

TEST_P(ThreadSafeQueueFixtureWithParams, ConcurrentPushAndWaitAndPopTest)
{
    static_assert(std::atomic_size_t::is_always_lock_free);

    auto params = GetParam();

    std::atomic_size_t actuallyValuesCount = 0;

    std::atomic_bool startFlag = false;

    auto producer = [&]() {
        startFlag.wait(false, std::memory_order::acquire);

        std::ranges::for_each(std::views::iota(0u, params.dataCount), [&](auto i) {
            queue.Push(i);
        });
    };

    std::vector<std::future<void>> futures;
    futures.reserve(params.producerCount + params.consumerCount);

    for (auto _ : std::views::iota(0u, params.producerCount))
    {
        futures.emplace_back(std::async(std::launch::async, producer));
    }

    auto consumer = [&]() {
        startFlag.wait(false, std::memory_order::acquire);
        do
        {
            int res = 0;
            queue.WaitAndPop(res);
        } while (actuallyValuesCount.fetch_add(1, std::memory_order_relaxed) + 1 < params.dataCount * params.producerCount);

        queue.Shutdown();

    };

    for (auto _ : std::views::iota(0u, params.consumerCount))
    {
        futures.emplace_back(std::async(std::launch::async, consumer));
    }

    PerformanceTest<std::chrono::milliseconds>(params.name, [&]() {
        startFlag.store(true, std::memory_order_release);
        startFlag.notify_all();

        std::ranges::for_each(futures, [](auto& fut) {
            fut.wait();
        });
    });

    ASSERT_EQ(actuallyValuesCount.load(std::memory_order_relaxed), params.dataCount * params.producerCount);
}

INSTANTIATE_TEST_SUITE_P(
    PerformanceTests,
    ThreadSafeQueueFixtureWithParams,
    ::testing::Values(
        PerfTestParams{.name = "1p_1c_1kk", .dataCount = 1'000'000, .producerCount = 1, .consumerCount = 1},
        PerfTestParams{.name = "2p_1c_1kk", .dataCount = 1'000'000, .producerCount = 2, .consumerCount = 1},
        PerfTestParams{.name = "5p_1c_1kk", .dataCount = 1'000'000, .producerCount = 5, .consumerCount = 1},

        PerfTestParams{.name = "1p_2c_1kk", .dataCount = 1'000'000, .producerCount = 1, .consumerCount = 2},
        PerfTestParams{.name = "1p_5c_1kk", .dataCount = 1'000'000, .producerCount = 1, .consumerCount = 5},
        PerfTestParams{.name = "1p_10c_1kk", .dataCount = 1'000'000, .producerCount = 1, .consumerCount = 10},
        PerfTestParams{.name = "1p_16c_1kk", .dataCount = 1'000'000, .producerCount = 1, .consumerCount = 16},

        PerfTestParams{.name = "2p_2c_1kk", .dataCount = 1'000'000, .producerCount = 2, .consumerCount = 2},
        PerfTestParams{.name = "5p_5c_1kk", .dataCount = 1'000'000, .producerCount = 5, .consumerCount = 5}));
