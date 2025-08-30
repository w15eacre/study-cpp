#include <algorithm>
#include <future>
#include <ranges>

#include <benchmark/benchmark.h>

import threadsafe_queue;
import threadsafe_queue_mutex;
import threadsafe_std_queue;

namespace {

template <typename Queue>
void PerfTest(benchmark::State& state)
{
    for (auto _ : state)
    {
        Queue queue{};

        static_assert(std::atomic_size_t::is_always_lock_free);

        std::atomic_size_t actuallyValuesCount = 0;

        std::atomic_bool startFlag = false;

        auto dataCount = static_cast<uint64_t>(state.range(0));
        auto producerCount = static_cast<uint64_t>(state.range(1));
        auto consumerCount = static_cast<uint64_t>(state.range(2));

        auto producer = [&]() {
            startFlag.wait(false, std::memory_order::acquire);

            std::ranges::for_each(std::views::iota(0u, dataCount), [&](auto i) {
                queue.Push(i);
            });
        };

        std::vector<std::future<void>> futures;
        futures.reserve(producerCount + consumerCount);

        for (auto _ : std::views::iota(0u, producerCount))
        {
            futures.emplace_back(std::async(std::launch::async, producer));
        }

        auto consumer = [&]() {
            startFlag.wait(false, std::memory_order::acquire);
            do
            {
                int res = 0;
                queue.WaitAndPop(res);
            } while (actuallyValuesCount.fetch_add(1, std::memory_order_relaxed) + 1 < dataCount * producerCount);

            queue.Shutdown();
        };

        for (auto _ : std::views::iota(0u, consumerCount))
        {
            futures.emplace_back(std::async(std::launch::async, consumer));
        }

        startFlag.store(true, std::memory_order_release);
        startFlag.notify_all();

        std::ranges::for_each(futures, [](auto& fut) {
            fut.wait();
        });

        assert(actuallyValuesCount.load(std::memory_order_relaxed) == dataCount * producerCount);

        const auto totalCount = double(dataCount) * double(producerCount);
        state.counters["items/s"] = benchmark::Counter(totalCount, benchmark::Counter::kIsRate);
        state.SetLabel((std::to_string(producerCount) + "P/" + std::to_string(consumerCount) + "C").c_str());
    }
}

} // namespace

BENCHMARK_TEMPLATE(PerfTest, threadsafe_queue::ThreadSafeStdQueue<int>)->ArgsProduct({{5'000'000}, {1, 2, 5, 10, 20}, {1, 2, 5, 10, 20}});
BENCHMARK_TEMPLATE(PerfTest, threadsafe_queue::ThreadSafeQueue<int>)->ArgsProduct({{5'000'000}, {1, 2, 5, 10, 20}, {1, 2, 5, 10, 20}});
BENCHMARK_TEMPLATE(PerfTest, threadsafe_queue::ThreadSafeQueueMutex<int>)->ArgsProduct({{5'000'000}, {1, 2, 5, 10, 20}, {1, 2, 5, 10, 20}});

BENCHMARK_MAIN();