#include <algorithm>
#include <future>
#include <ranges>

#include <gtest/gtest.h>

import threadsafe_queue;
import threadsafe_queue_mutex;
import threadsafe_std_queue;

namespace {

struct PerfTestParams
{
    std::string_view name{};
    size_t dataCount{};
    size_t producerCount{};
    size_t consumerCount{};
};

class ThreadSafeQueueFixtureWithParams : public ::testing::TestWithParam<PerfTestParams>
{
public:
    ~ThreadSafeQueueFixtureWithParams() noexcept = default;
protected:
    threadsafe_queue::ThreadSafeQueue<int> queue{};
};

class ThreadSafeQueueMutexFixtureWithParams : public ::testing::TestWithParam<PerfTestParams>
{
public:
    ~ThreadSafeQueueMutexFixtureWithParams() noexcept = default;
protected:
    threadsafe_queue::ThreadSafeQueueMutex<int> queue{};
};

class ThreadSafeStdQueueFixtureWithParams : public ::testing::TestWithParam<PerfTestParams>
{
public:
    ~ThreadSafeStdQueueFixtureWithParams() noexcept = default;
protected:
    threadsafe_queue::ThreadSafeStdQueue<int> queue{};
};

template <typename Queue>
void PerfTest(const PerfTestParams& params, Queue& queue)
{
    static_assert(std::atomic_size_t::is_always_lock_free);

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

    startFlag.store(true, std::memory_order_release);
    startFlag.notify_all();

    std::ranges::for_each(futures, [](auto& fut) {
        fut.wait();
    });

    ASSERT_EQ(actuallyValuesCount.load(std::memory_order_relaxed), params.dataCount * params.producerCount);
}

} // namespace

TEST_P(ThreadSafeQueueFixtureWithParams, ConcurrentPushAndWaitAndPopTest)
{
    PerfTest(GetParam(), queue);
}

TEST_P(ThreadSafeQueueMutexFixtureWithParams, ConcurrentPushAndWaitAndPopTest)
{
    PerfTest(GetParam(), queue);
}

TEST_P(ThreadSafeStdQueueFixtureWithParams, ConcurrentPushAndWaitAndPopTest)
{
    PerfTest(GetParam(), queue);
}

static auto PerfParams = ::testing::Values(
    PerfTestParams{.name = "1p_1c_5kk", .dataCount = 5'000'000, .producerCount = 1, .consumerCount = 1},
    PerfTestParams{.name = "2p_1c_10kk", .dataCount = 5'000'000, .producerCount = 2, .consumerCount = 1},
    PerfTestParams{.name = "5p_1c_25kk", .dataCount = 5'000'000, .producerCount = 5, .consumerCount = 1},
    PerfTestParams{.name = "10p_1c_50kk", .dataCount = 5'000'000, .producerCount = 10, .consumerCount = 1},
    PerfTestParams{.name = "20p_1c_100kk", .dataCount = 5'000'000, .producerCount = 20, .consumerCount = 1},

    PerfTestParams{.name = "1p_2c_5kk", .dataCount = 5'000'000, .producerCount = 1, .consumerCount = 2},
    PerfTestParams{.name = "1p_5c_5kk", .dataCount = 5'000'000, .producerCount = 1, .consumerCount = 5},
    PerfTestParams{.name = "1p_10c_5kk", .dataCount = 5'000'000, .producerCount = 1, .consumerCount = 10},
    PerfTestParams{.name = "1p_20c_5kk", .dataCount = 5'000'000, .producerCount = 1, .consumerCount = 20},

    PerfTestParams{.name = "2p_2c_10kk", .dataCount = 5'000'000, .producerCount = 2, .consumerCount = 2},
    PerfTestParams{.name = "5p_5c_25kk", .dataCount = 5'000'000, .producerCount = 5, .consumerCount = 5},
    PerfTestParams{.name = "10p_10c_50kk", .dataCount = 5'000'000, .producerCount = 10, .consumerCount = 10},
    PerfTestParams{.name = "20p_20c_100kk", .dataCount = 5'000'000, .producerCount = 20, .consumerCount = 20});

INSTANTIATE_TEST_SUITE_P(PerformanceTests, ThreadSafeQueueFixtureWithParams, PerfParams);
INSTANTIATE_TEST_SUITE_P(PerformanceTests, ThreadSafeQueueMutexFixtureWithParams, PerfParams);
INSTANTIATE_TEST_SUITE_P(PerformanceTests, ThreadSafeStdQueueFixtureWithParams, PerfParams);