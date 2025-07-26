module;

#include <memory>
#include <mutex>
#include <utility>

export module thread_safe_list;

import thread_safe_list_concept;

export namespace threadsafe_list {

template <typename T>
class ThreadSafeList
{
public:
    ThreadSafeList() : m_head(std::make_unique<Node>()) {}

    ThreadSafeList(const ThreadSafeList &) = delete;
    ThreadSafeList(ThreadSafeList &&) noexcept = delete;
    ThreadSafeList &operator=(const ThreadSafeList &) = delete;
    ThreadSafeList &operator=(ThreadSafeList &&) = delete;

    ~ThreadSafeList() = default;

    void PushFront(T value)
    {
        auto newNode = std::make_unique<Node>(std::move(value));
        {
            std::scoped_lock lock{m_head->mutex};
            newNode->next = std::move(m_head->next);
            m_head->next = std::move(newNode);
        }
    };

    template <AccessFunctionConcept<T> Func>
    void ForEach(const Func &func)
    {
        Node *node = m_head.get();

        std::unique_lock lock{node->mutex};
        while ((node = node->next.get()))
        {
            std::unique_lock nextLock{node->mutex};
            lock = std::move(nextLock);

            func(*node->data);
        }
    }

    template <PredicateConcept<T> Func>
    std::shared_ptr<T> FindFirstIf(const Func &pred)
    {
        Node *node = m_head.get();

        std::unique_lock lock{node->mutex};
        while ((node = node->next.get()))
        {
            auto data = node->data;
            std::unique_lock nextLock{node->mutex};
            lock = std::move(nextLock);

            if (pred(*data))
            {
                return data;
            }
        }

        return {};
    }

    template <typename Func>
    void RemoveIf([[maybe_unused]] const Func &pred)
    {
        Node *current = m_head.get();

        std::unique_lock lock{current->mutex};
        while (auto next = current->next.get())
        {
            std::unique_lock nextLock(next->mutex);

            if (pred(*next->data))
            {
                current->next = std::move(next->next);
            }
            else
            {
                lock = std::move(nextLock);
                current = next;
            }
        }
    };
private:
    struct Node
    {
        Node() : data(nullptr), next(nullptr) {}
        Node(T value) : data(std::make_shared<T>(std::move(value))), next(nullptr) {}

        std::mutex mutex;
        std::shared_ptr<T> data;
        std::unique_ptr<Node> next;
    };

    std::unique_ptr<Node> m_head{std::make_unique<Node>()};
};

static_assert(ThreadSafeListConcept<ThreadSafeList, int>, "ThreadSafeList does not satisfy the ThreadSafeListConcept concept");

} // namespace threadsafe_list
