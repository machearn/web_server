#include <condition_variable>
#include <cstdint>
#include <exception>
#include <functional>
#include <future>
#include <memory>
#include <mutex>
#include <queue>
#include <thread>
#include <type_traits>
#include <utility>

namespace web_server {
namespace thread {

class ThreadPool {
public:
  ThreadPool(std::uint32_t thread_count)
      : _thread_count(_determin_thread_count(thread_count)),
        _threads(std::make_unique<std::thread[]>(_determin_thread_count(thread_count))) {
    _create_threads();
  }
  ~ThreadPool() { destroy(); }

  void pause() {
    std::scoped_lock<std::mutex> lock{_task_mutex};
    _pause = true;
  }

  void purge() {
    std::scoped_lock<std::mutex> lock{_task_mutex};
    while (!_tasks.empty()) {
      _tasks.pop();
    }
  }

  template <typename F, typename... Args>
  void push_task(F&& f, Args&&... args) {
    {
      std::scoped_lock<std::mutex> lock{_task_mutex};
      _tasks.emplace(std::bind(std::forward<F>(f), std::forward<Args>(args)...));
    }
    _task_available_cv.notify_one();
  }

  template <typename F, typename... Args,
            typename R = std::invoke_result_t<std::decay_t<F>, std::decay_t<Args>...>>
  [[nodiscard]] std::future<R> submit(F&& task, Args&&... args) {
    std::shared_ptr<std::promise<R>> promise = std::make_shared<std::promise<R>>();
    push_task(
        [task_function = std::bind(std::forward<F>(task), std::forward<Args>(args)...), promise]() {
          try {
            if constexpr (std::is_void_v<R>) {
              std::invoke(task_function);
              promise->set_value();
            } else {
              promise->set_value(std::invoke(task_function));
            }
          } catch (...) {
            try {
              promise->set_exception(std::current_exception());
            } catch (...) {
            }
          }
        });
    return promise->get_future();
  }

  void wait_for_tasks() {
    std::unique_lock<std::mutex> lock{_task_mutex};
    _wait_for_tasks = true;
    _task_done_cv.wait(lock,
                       [this]() { return _running_tasks == 0 && (_pause || _tasks.empty()); });
    _wait_for_tasks = false;
  }

  void destroy() {
    wait_for_tasks();
    _destroy_threads();
  }

private:
  void _create_threads() {
    {
      const std::scoped_lock<std::mutex> lock{_task_mutex};
      _workers_running = true;
    }

    for (std::uint32_t i = 0; i < _thread_count; i++) {
      _threads[i] = std::thread{&ThreadPool::_worker, this};
    }
  }

  void _destroy_threads() {
    {
      const std::scoped_lock<std::mutex> lock{_task_mutex};
      _workers_running = false;
    }

    _task_available_cv.notify_all();

    for (std::uint32_t i = 0; i < _thread_count; i++) {
      if (_threads[i].joinable()) {
        _threads[i].join();
      }
    }
  }

  [[nodiscard]] std::uint32_t _determin_thread_count(std::uint32_t thread_count) const noexcept {
    if (thread_count > 0) {
      return thread_count;
    } else {
      thread_count = std::thread::hardware_concurrency();
      return thread_count > 0 ? thread_count : 1;
    }
  }

  void _worker() {
    std::function<void()> task{};
    while (true) {
      std::unique_lock<std::mutex> lock{_task_mutex};
      _task_available_cv.wait(lock, [this]() { return !_tasks.empty() || !_workers_running; });

      if (!_workers_running) {
        return;
      }
      if (_pause) {
        continue;
      }

      task = std::move(_tasks.front());
      _tasks.pop();
      ++_running_tasks;
      lock.unlock();
      task();
      lock.lock();
      --_running_tasks;

      if (_wait_for_tasks && _running_tasks == 0 && (_pause || _tasks.empty())) {
        _task_done_cv.notify_all();
      }
    }
  }

  bool _pause{};
  bool _workers_running{false};
  bool _wait_for_tasks{false};

  std::condition_variable _task_available_cv{};
  std::condition_variable _task_done_cv{};

  mutable std::mutex _task_mutex{};

  std::uint32_t _thread_count{};
  std::uint32_t _running_tasks{};
  std::unique_ptr<std::thread[]> _threads{};
  std::queue<std::function<void()>> _tasks{};
};

} // namespace thread
} // namespace web_server
