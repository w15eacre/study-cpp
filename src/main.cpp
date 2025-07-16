#include <format>
#include <iostream>

import threadsafe_queue;

int main()
{
    threadsafe_queue::ThreadSafeQueue<int> queue;

    queue.Push(10);

    int res = 0;
    queue.Pop(res);

    std::cout << std::format("Pushed value: {}\n", res) << std::endl;

    return 0;
}