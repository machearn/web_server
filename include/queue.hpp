#ifndef QUEUE_H_
#define QUEUE_H_

#include <condition_variable>
#include <mutex>
#include <queue>

namespace web_server {
namespace utils {

template <typename T>
class Queue {
public:
  Queue() = default;
  Queue(const Queue<T>&) = delete;
  Queue& operator=(const Queue<T>&) = delete;

  Queue(Queue<T>&& other_queue) {
    std::lock_guard<std::mutex> lock{_mutex};
    _queue = std::move(other_queue._queue);
  }

  Queue<T>& operator=(Queue<T>&& other_queue) {
    std::lock_guard<std::mutex> lock{_mutex};
    _queue = std::move(other_queue._queue);
    return *this;
  }

  ~Queue() = default;

  void push(const T& value) {
    std::lock_guard<std::mutex> lock{_mutex};
    _queue.push(value);
    _cv.notify_one();
  }

  void push(T&& value) {
    std::lock_guard<std::mutex> lock{_mutex};
    _queue.push(std::move(value));
    _cv.notify_one();
  }

  int pop(T& value) {
    std::unique_lock<std::mutex> lock{_mutex};
    _cv.wait_for(lock, std::chrono::seconds(1), [this]() { return !_queue.empty(); });
    if (_queue.empty()) {
      return -1;
    }
    value = std::move(_queue.front());
    _queue.pop();
    return 0;
  }

  T pop() {
    std::unique_lock<std::mutex> lock{_mutex};
    _cv.wait(lock, [this]() { return !_queue.empty(); });
    T value = std::move(_queue.front());
    _queue.pop();
    return value;
  }

  bool empty() const {
    std::lock_guard<std::mutex> lock{_mutex};
    return _queue.empty();
  }

  std::size_t size() const {
    std::lock_guard<std::mutex> lock{_mutex};
    return _queue.size();
  }

  void clear() {
    std::lock_guard<std::mutex> lock{_mutex};
    while (!_queue.empty()) {
      _queue.pop();
    }
  }

private:
  std::queue<T> _queue;
  mutable std::mutex _mutex;
  std::condition_variable _cv;
};

} // namespace utils
} // namespace web_server

#endif // QUEUE_H_
