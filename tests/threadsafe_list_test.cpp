#include <algorithm>
#include <chrono>
#include <future>
#include <ranges>

#include <gtest/gtest.h>

import thread_safe_list;

namespace {

template <typename List>
class ThreadSafeListTypedFixture : public ::testing::Test
{
public:
    ~ThreadSafeListTypedFixture() noexcept = default;
    using ListType = List;
};

} // namespace

using ListTypes = ::testing::Types<threadsafe_list::ThreadSafeList<int>>;
TYPED_TEST_SUITE(ThreadSafeListTypedFixture, ListTypes);

TYPED_TEST(ThreadSafeListTypedFixture, PushFront_ExpectAddedToFront)
{
    typename TestFixture::ListType list{};

    list.PushFront(42);
    list.PushFront(24);

    std::vector<int> results;
    list.ForEach([&results](const int &value) {
        results.push_back(value);
    });

    ASSERT_EQ(results.size(), 2);
    ASSERT_EQ(results[0], 24);
    ASSERT_EQ(results[1], 42);
}

TYPED_TEST(ThreadSafeListTypedFixture, FindFirstIf_ExpectFound)
{
    typename TestFixture::ListType list{};

    list.PushFront(42);
    list.PushFront(24);
    list.PushFront(100);

    auto found = list.FindFirstIf([](const int &value) {
        return value == 24;
    });

    ASSERT_TRUE(found);
    ASSERT_EQ(*found, 24);
}

TYPED_TEST(ThreadSafeListTypedFixture, RemoveIf_ExpectRemoved)
{
    typename TestFixture::ListType list{};

    list.PushFront(42);
    list.PushFront(24);
    list.PushFront(100);

    list.RemoveIf([](const int &value) {
        return value == 24;
    });

    std::vector<int> results;
    list.ForEach([&results](const int &value) {
        results.push_back(value);
    });

    ASSERT_EQ(results.size(), 2);
    ASSERT_EQ(results[0], 100);
    ASSERT_EQ(results[1], 42);
}

TYPED_TEST(ThreadSafeListTypedFixture, ForEach_ExpectAllElementsVisited)
{
    typename TestFixture::ListType list{};

    list.PushFront(1);
    list.PushFront(2);
    list.PushFront(3);

    std::vector<int> results;
    list.ForEach([&results](const int &value) {
        results.push_back(value);
    });

    ASSERT_EQ(results.size(), 3);
    ASSERT_TRUE(std::ranges::is_sorted(results, std::greater<>()));
}
