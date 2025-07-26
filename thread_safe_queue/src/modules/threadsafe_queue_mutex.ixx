module;

#include <condition_variable>
#include <memory>
#include <mutex>

export module threadsafe_queue_mutex;

import threadsafe_queue_concept;

export namespace threadsafe_queue {

template <typename T>
class ThreadSafeQueueMutex
{
public:
    ThreadSafeQueueMutex() : m_head(std::make_unique<Node>()), m_tail(m_head.get()) {};

    ThreadSafeQueueMutex(const ThreadSafeQueueMutex &other) = delete;
    ThreadSafeQueueMutex(ThreadSafeQueueMutex &&other) noexcept = delete;
    ThreadSafeQueueMutex &operator=(const ThreadSafeQueueMutex &other) = delete;
    ThreadSafeQueueMutex &operator=(ThreadSafeQueueMutex &&) = delete;

    ~ThreadSafeQueueMutex() noexcept(false)
    {
        Shutdown();
    }

    void Push(T value)
    {
        auto newNode = std::make_unique<Node>(std::move(value));
        {
            std::scoped_lock lock{m_mutex};
            CheckStopped();

            m_tail->next = std::move(newNode);
            m_tail = m_tail->next.get();
        }

        m_conditionVariable.notify_one();
    }

    void Pop(T &value)
    {
        std::scoped_lock lock{m_mutex};
        auto headValue = HeadData();
        if (!headValue)
        {
            throw std::runtime_error("ThreadSafeQueueMutex is empty");
        }

        value = std::move(*headValue);
        m_head = std::move(m_head->next);
    }

    bool TryPop(T &value)
    {
        std::scoped_lock lock{m_mutex};
        auto headValue = HeadData();
        if (!headValue)
        {
            return false;
        }

        value = std::move(*headValue);
        m_head = std::move(m_head->next);
        return true;
    }

    void WaitAndPop(T &value)
    {
        std::unique_lock lock{m_mutex};
        m_conditionVariable.wait(lock, [this]() {
            return !IsEnd() || m_stopped;
        });

        CheckStopped();

        value = std::move(*HeadData());
        m_head = std::move(m_head->next);
    }

    void Shutdown()
    {
        {
            // Shutdown must acquire m_headMutex to avoid a race condition with WaitAndPop.
            //
            // Scenario without the mutex:
            //
            // Thread 1 (consumer):
            //   - Acquires m_headMutex
            //   - Evaluates the predicate: (m_stopped == false && queue is empty)
            //   - Predicate returns false, so it proceeds to wait
            //
            // Thread 2 (Shutdown):
            //   - Sets m_stopped = true
            //   - Calls notify_all()
            //
            // Now, Thread 1 goes to sleep — but it has already missed the notification.
            // Result: Thread 1 will sleep forever.
            //
            // By holding m_headMutex in Shutdown, we guarantee that:
            //
            //   - Either the consumer sees m_stopped == true before waiting,
            //   - Or it begins waiting only after notify_all() is issued.
            //
            // This ensures that no thread can miss the notification signal due to
            // a race between the predicate check and notify_all().

            std::unique_lock lock{m_mutex};
            m_stopped = true;
        }

        m_conditionVariable.notify_all();
    }
private:
    struct Node
    {
        Node() : data(nullptr), next(nullptr) {}
        Node(T value) : data(std::make_unique<T>(std::move(value))), next(nullptr) {}

        std::unique_ptr<T> data{};
        std::unique_ptr<Node> next{};
    };

    const Node *Tail() const
    {
        return m_tail;
    }

    const T *HeadData() const
    {
        return IsEnd() ? nullptr : m_head->next->data.get();
    }

    bool IsEnd() const
    {
        return m_head.get() == Tail();
    }

    void CheckStopped() const
    {
        if (m_stopped)
        {
            throw std::runtime_error("ThreadSafeQueueMutex has been stopped");
        }
    }
private:
    mutable std::mutex m_mutex{};
    std::unique_ptr<Node> m_head{};
    Node *m_tail{};

    std::condition_variable m_conditionVariable{};
    bool m_stopped{false};
};

static_assert(ThreadSafeQueueConcept<int, ThreadSafeQueueMutex>, "ThreadSafeQueueMutex does not satisfy the ThreadSafeQueueConcept");

} // namespace threadsafe_queue
