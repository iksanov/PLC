#pragma once

#include <exception>
#include <deque>
#include <mutex>
#include <condition_variable>
#include <functional>
#include <future>
#include <vector>

#include "blocking_queue.h"
#include "future_promise.h"


template <class T>
class ThreadPool {
 public:
  ThreadPool();
  explicit ThreadPool(size_t num_threads);
  Future<T> Submit(std::function<T()> task);
  void Shutdown();
  std::atomic<size_t> num_free_threads_;
  ~ThreadPool();
 private:
  bool is_shutdown_;
  size_t default_num_workers();
  size_t num_workers_;
  std::vector<std::thread> worker_threads_;
  BlockingQueue<std::function<void()>> tasks_;
};

template <class T>
size_t ThreadPool<T>::default_num_workers() {
  size_t num_workers = std::thread::hardware_concurrency();
  if (num_workers > 0) {
    return num_workers;
  }
  return 2;
}

template <class T>
ThreadPool<T>::ThreadPool()
    : is_shutdown_(false),
      num_workers_(default_num_workers()),
      worker_threads_(std::vector<std::thread>(default_num_workers())),
      tasks_(default_num_workers()),
      num_free_threads_(default_num_workers()) {
  for (auto& worker_thread : worker_threads_) {
    worker_thread = std::thread([&]() {
      std::function<void()> fun;
      while (tasks_.Get(fun)) {
        num_free_threads_.fetch_sub(1);
        fun();
        num_free_threads_.fetch_sub(1);
      }
    });
  }
}

template <class T>
ThreadPool<T>::ThreadPool(const size_t num_threads)
    : is_shutdown_(false),
      num_workers_(num_threads),
      worker_threads_(std::vector<std::thread>(num_threads)),
      tasks_(num_threads),
      num_free_threads_(num_threads) {
  for (auto& worker_thread : worker_threads_) {
    worker_thread = std::thread([&]() {
      std::function<void()> fun;
      while (tasks_.Get(fun)) {
        num_free_threads_.fetch_sub(1);
        fun();
        num_free_threads_.fetch_add(1);
      }
    });
  }
}

template <class T>
Future<T> ThreadPool<T>::Submit(std::function<T()> task) {
  Promise<T> promise;
  tasks_.Put([promise, task]() mutable {
    try {
      promise.SetValue(task());
    } catch(const std::exception &excep) {
      promise.SetException(excep);
    }
  });
  return promise.getFuture();
}

template <class T>
void ThreadPool<T>::Shutdown() {
  tasks_.Shutdown();
  is_shutdown_ = true;
  for (auto& worker_thread : worker_threads_) {
    worker_thread.join();
  }
}

template <class T>
ThreadPool<T>::~ThreadPool() {
  if (!is_shutdown_) {
    Shutdown();
  }
}
