module;

#include <atomic>
#include <condition_variable>
#include <memory>
#include <mutex>

export module threadsafe_queue;

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

    ~ThreadSafeQueue() noexcept
    {
        Shutdown();
    }

    void Push(T value)
    {
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
        std::ignore = PopHead();
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

        return PopHead();
    }

    void WaitAndPop(T &value)
    {
        std::unique_lock lock{m_headMutex};
        m_conditionVariable.wait(lock, [this]() {
            return m_stopped.load(std::memory_order_acquire) || m_head != Tail();
        });

        if (m_stopped)
        {
            throw std::runtime_error("ThreadSafeQueue is stopped");
        }

        value = std::move(*HeadData());
        PopHead();
    }

    void Shutdown() noexcept
    {
        m_stopped.store(true, std::memory_order_release);
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

    bool PopHead()
    {
        if (m_head.get() == Tail())
        {
            return false;
        }

        m_head = std::move(m_head->next);

        return true;
    }

    const T *HeadData() const
    {
        if (m_head.get() == Tail())
        {
            return nullptr;
        }

        return m_head->next->data.get();
    }
private:
    mutable std::mutex m_headMutex{};
    std::unique_ptr<Node> m_head{};

    mutable std::mutex m_tailMutex{};
    Node *m_tail{};

    std::condition_variable m_conditionVariable{};
    std::atomic<bool> m_stopped{false};
};

} // namespace threadsafe_queue
