module;

#include <atomic>
#include <condition_variable>
#include <memory>
#include <mutex>

export module threadsafe_queue;

import threadsafe_queue_concept;

export namespace threadsafe_queue {

template <typename T>
class ThreadSafeQueue
{
public:
    ThreadSafeQueue() : m_head(std::make_unique<Node>()), m_tail(m_head.get()) {};

    ThreadSafeQueue(const ThreadSafeQueue &other) = delete;
    ThreadSafeQueue(ThreadSafeQueue &&other) noexcept = delete;
    ThreadSafeQueue &operator=(const ThreadSafeQueue &other) = delete;
    ThreadSafeQueue &operator=(ThreadSafeQueue &&) = delete;

    ~ThreadSafeQueue() noexcept(false)
    {
        Shutdown();
    }

    void Push(T value)
    {
        CheckStopped();

        auto newNode = std::make_unique<Node>(std::move(value));
        {
            std::scoped_lock lock{m_tailMutex};
            m_tail->next = std::move(newNode);
            m_tail = m_tail->next.get();
        }

        m_conditionVariable.notify_one();
    }

    void Pop(T &value)
    {
        std::scoped_lock lock{m_headMutex};
        auto headValue = HeadData();
        if (!headValue)
        {
            throw std::runtime_error("ThreadSafeQueue is empty");
        }

        value = std::move(*headValue);
        m_head = std::move(m_head->next);
    }

    bool TryPop(T &value)
    {
        std::scoped_lock lock{m_headMutex};
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
        std::unique_lock lock{m_headMutex};
        m_conditionVariable.wait(lock, [this]() {
            return !IsEnd() || m_stopped.load(std::memory_order::acquire);
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

            std::unique_lock lock{m_headMutex};
            m_stopped.store(true, std::memory_order_release);
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
        std::scoped_lock lock{m_tailMutex};
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
        if (m_stopped.load(std::memory_order::acquire))
        {
            throw std::runtime_error("ThreadSafeQueue has been stopped");
        }
    }
private:
    mutable std::mutex m_headMutex{};
    std::unique_ptr<Node> m_head{};

    mutable std::mutex m_tailMutex{};
    Node *m_tail{};

    std::condition_variable m_conditionVariable{};
    std::atomic<bool> m_stopped{false};
};

static_assert(ThreadSafeQueueConcept<int, ThreadSafeQueue>, "ThreadSafeQueue does not satisfy the ThreadSafeQueueConcept");

} // namespace threadsafe_queue
