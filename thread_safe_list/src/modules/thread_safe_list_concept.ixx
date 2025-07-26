module;

#include <concepts>
#include <functional>
#include <memory>
#include <type_traits>

export module thread_safe_list_concept;

export namespace threadsafe_list {

template <typename Fn, typename T>
concept PredicateConcept = std::is_invocable_r<bool, Fn, T>::value;

template <typename Fn, typename T>
concept AccessFunctionConcept = std::invocable<Fn, T>;

template <template <typename> class ListType, typename T>
concept ThreadSafeListConcept = requires(ListType<T> list, const T &value) {
    { list.PushFront(value) } -> std::same_as<void>;
    { list.ForEach(std::declval<std::function<void(const T &)>>()) } -> std::same_as<void>;
    { list.FindFirstIf(std::declval<std::function<bool(const T &)>>()) } -> std::same_as<std::shared_ptr<T>>;
    { list.RemoveIf(std::declval<std::function<bool(const T &)>>()) } -> std::same_as<void>;
};

}; // namespace thread_safe_list
