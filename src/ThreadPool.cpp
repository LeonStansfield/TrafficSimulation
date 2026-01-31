#include "ThreadPool.hpp"

ThreadPool::ThreadPool(size_t threads) : stop(false) {
  // Launch the specified number of worker threads
  for (size_t i = 0; i < threads; ++i)
    workers.emplace_back([this] {
      // Infinite loop ensures the thread stays alive to process multiple tasks
      for (;;) {
        std::function<void()> task;

        {
          // Acquire the lock to safely access the task queue
          std::unique_lock<std::mutex> lock(this->queue_mutex);

          // Wait until there is a task OR we are stopping.
          // This releases the lock and puts the thread to sleep to save CPU.
          this->condition.wait(
              lock, [this] { return this->stop || !this->tasks.empty(); });

          // If stopping and no tasks left, exit the thread function
          if (this->stop && this->tasks.empty())
            return;

          // Retrieve the next task
          task = std::move(this->tasks.front());
          this->tasks.pop();
        }

        // Execute the task outside the lock so other threads can access the
        // queue
        task();
      }
    });
}

ThreadPool::~ThreadPool() {
  {
    std::unique_lock<std::mutex> lock(queue_mutex);
    stop = true;
  }
  condition.notify_all();
  for (std::thread &worker : workers)
    worker.join();
}
