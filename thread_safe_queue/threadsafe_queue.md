## 📦 ThreadSafeQueue

🔒 A thread-safe queue implementation built with `std::mutex`.

* Written as a **C++20 module**;
* Provides `push`, `try_pop`, `wait_and_pop` interfaces;
* Fully tested using **GoogleTest**.

> 📖 Inspired by "C++ Concurrency in Action" by Anthony Williams.

### Example

```cpp
ThreadSafeQueue<int> queue;

queue.Push(42);
int value{};
if (queue.TryPop(value)) {
    std::cout << "Got: " << value << std::endl;
}
```

---

## 🚀 Queue Performance Comparison: ThreadSafeQueue vs ThreadSafeQueueMutex vs ThreadSafeStdQueue

Benchmarks comparing three thread-safe queue implementations under various producer/consumer loads.

### 🖥️ System Specs

* **Architecture:** x86\_64
* **CPU:** AMD Ryzen 7 5800X (8 cores, 16 threads)
* **RAM:** 32 GB DDR4
* **OS:** Debian 12

### 🧩 Implementations

1. **ThreadSafeQueue**

   * Custom singly-linked queue;
   * Two mutexes: separate locks for front and back;
   * Reduces producer/consumer contention.

2. **ThreadSafeQueueMutex**

   * Same structure as above, but with a single shared mutex;
   * Simpler design, but more contention.

3. **ThreadSafeStdQueue**

   * Wraps `std::queue<T>`;
   * Uses one mutex;
   * Easy to implement, good performance in light-to-moderate concurrency.

### 📊 Performance Table (ms)

> **Legend**:
>
> * `p` — number of **producers**
> * `c` — number of **consumers**
>
> For example, `5p_10c` means 5 producer threads and 10 consumer threads.

| Test Case | ThreadSafeQueue (2 mutexes) | ThreadSafeQueueMutex (1 mutex) | ThreadSafeStdQueue (std::queue + 1 mutex) |
| --------- | --------------------------- | ------------------------------ | ----------------------------------------- |
| 1p\_1c    | 2201                        | 4001                           | 1414                                      |
| 2p\_1c    | 4337                        | 5429                           | 1838                                      |
| 5p\_1c    | 11238                       | 11488                          | 5173                                      |
| 10p\_1c   | 24641                       | 24083                          | 12562                                     |
| 20p\_1c   | 52861                       | 50178                          | 27393                                     |
| 1p\_2c    | 3215                        | 5308                           | 1213                                      |
| 1p\_5c    | 3699                        | 8616                           | 3600                                      |
| 1p\_10c   | 3672                        | 8800                           | 6280                                      |
| 1p\_20c   | 3960                        | 8184                           | 5580                                      |
| 2p\_2c    | 6921                        | 6737                           | 2441                                      |
| 5p\_5c    | 20129                       | 20751                          | 10541                                     |
| 10p\_10c  | 45931                       | 66592                          | 53937                                     |
| 20p\_20c  | 98990                       | 153844                         | 101619                                    |

### 🔎 Observations

* ✅ `ThreadSafeStdQueue` is the fastest across most scenarios, especially low-concurrency.
* ✅ `ThreadSafeQueue` provides balanced performance with less contention.
* 🚫 `ThreadSafeQueueMutex` consistently performs worst under high load.

### 🔁 Validation Note

All tests were repeated, and results were consistent. The original conclusions are confirmed.
