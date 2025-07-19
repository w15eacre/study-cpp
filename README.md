# study-cpp 🧠💻

**An educational C++ repository exploring multithreading, C++20 features, and modern development practices.**  
This project was created for self-learning purposes — as I read books and study new topics, I implement examples and write tests.

---

## 📚 About the Project

This repository contains concise and meaningful examples developed while learning:

- modern **C++20** features (including **modules**);
- working with **threads**, **mutexes**, and synchronization primitives;
- writing robust and clean unit tests with **GoogleTest**;
- building **safe and performant concurrent code**.

---

## 📦 ThreadSafeQueue

🔒 A thread-safe queue implemented using `std::mutex`.

- Written as a **C++20 module**
- Supports `push`, `try_pop`, `wait_and_pop`
- Covered with **unit tests** using GoogleTest

> 📖 Inspired by examples from books on C++ concurrency (e.g. *C++ Concurrency in Action* by Anthony Williams).

### Example usage

```cpp
ThreadSafeQueue<int> queue;

queue.Push(42);
int value{};
if (queue.TryPop(value)) {
    std::cout << "Got: " << value << std::endl;
}
