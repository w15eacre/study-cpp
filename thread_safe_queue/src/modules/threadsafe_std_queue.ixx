module;

#include <condition_variable>
#include <mutex>
#include <queue>

export module threadsafe_std_queue;

import threadsafe_queue_concept;

export namespace threadsafe_queue {

template <typename T>
class ThreadSafeStdQueue
{
public:
    ThreadSafeStdQueue() = default;

    ThreadSafeStdQueue(const ThreadSafeStdQueue &other) = delete;
    ThreadSafeStdQueue(ThreadSafeStdQueue &&other) noexcept = delete;
    ThreadSafeStdQueue &operator=(const ThreadSafeStdQueue &other) = delete;
    ThreadSafeStdQueue &operator=(ThreadSafeStdQueue &&) = delete;

    ~ThreadSafeStdQueue() noexcept(false)
    {
        Shutdown();
    }

    void Push(T value)
    {
        {
            std::unique_lock lock{m_mutex};
            CheckStopped();

            m_queue.push(std::move(value));
        }

        m_conditionVariable.notify_one();
    }

    void Pop(T &value)
    {
        std::unique_lock lock{m_mutex};
        if (m_queue.empty())
        {
            throw std::runtime_error("ThreadSafeQueue is empty");
        }

        value = std::move(m_queue.front());
        m_queue.pop();
    }

    bool TryPop(T &value)
    {
        std::scoped_lock lock{m_mutex};
        if (m_queue.empty())
        {
            return false;
        }

        value = std::move(m_queue.front());
        m_queue.pop();
        return true;
    }

    void WaitAndPop(T &value)
    {
        std::unique_lock lock{m_mutex};
        m_conditionVariable.wait(lock, [this]() {
            return m_stopped || !m_queue.empty();
        });

        CheckStopped();

        value = std::move(m_queue.front());
        m_queue.pop();
    }

    void Shutdown()
    {
        {
            std::unique_lock lock{m_mutex};
            m_stopped = true;
        }

        m_conditionVariable.notify_all();
    }
private:
    void CheckStopped() const
    {
        if (m_stopped)
        {
            throw std::runtime_error("ThreadSafeQueue has been stopped");
        }
    }
private:
    mutable std::mutex m_mutex{};
    std::queue<T> m_queue{};
    bool m_stopped{false};

    std::condition_variable m_conditionVariable{};
};

static_assert(ThreadSafeQueueConcept<int, ThreadSafeStdQueue>, "ThreadSafeStdQueue does not satisfy the ThreadSafeQueueConcept");

} // namespace threadsafe_queue
