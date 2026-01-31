#pragma once

#include <condition_variable>
#include <functional>
#include <future>
#include <mutex>
#include <queue>
#include <thread>
#include <type_traits>
#include <vector>

class ThreadPool {
public:
  // Constructor: Launches a fixed number of worker threads
  ThreadPool(size_t threads);

  // Destructor
  ~ThreadPool();

  // Enqueue a task to be executed by the thread pool
  // Template to accept any function and arguments
  template <class F, class... Args>
  auto enqueue(F &&f, Args &&...args)
      -> std::future<typename std::invoke_result<F, Args...>::type> {

    // Determine the return type of the function
    using return_type = typename std::invoke_result<F, Args...>::type;

    // Wrap the function and arguments into a shared_ptr packaged_task
    // This allows us to get a future return value later
    auto task = std::make_shared<std::packaged_task<return_type()>>(
        std::bind(std::forward<F>(f), std::forward<Args>(args)...));

    // Get the future to return to the caller
    std::future<return_type> res = task->get_future();
    {
      std::unique_lock<std::mutex> lock(queue_mutex);

      // Don't allow enqueueing after stopping
      if (stop)
        throw std::runtime_error("enqueue on stopped ThreadPool");

      // Add the task to the queue. The lambda wraps the shared_ptr task.
      tasks.emplace([task]() { (*task)(); });
    }

    // Wake up one waiting worker thread
    condition.notify_one();
    return res;
  }

private:
  // The worker threads constanty running
  std::vector<std::thread> workers;

  // The queue of tasks waiting to be picked up by workers
  std::queue<std::function<void()>> tasks;

  // Mutex to synchronize access to the task queue
  std::mutex queue_mutex;

  // Condition variable to let workers sleep when there are no tasks
  std::condition_variable condition;

  // Flag to tell workers to stop
  bool stop;
};
