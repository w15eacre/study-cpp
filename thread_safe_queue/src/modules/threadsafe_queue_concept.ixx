module;

#include <utility>

export module threadsafe_queue_concept;

export namespace threadsafe_queue {

template <typename T, template <typename> class Queue>
concept ThreadSafeQueueConcept = requires(Queue<T> queue, T value, T& ref) {
    { queue.Push(value) };
    { queue.Pop(ref) };
    { queue.TryPop(ref) } -> std::same_as<bool>;
    { queue.Shutdown() };
    { queue.WaitAndPop(ref) };
};

} // namespace threadsafe_queue
