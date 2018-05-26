#ifndef PROMISE_ASYNC_H
#define PROMISE_ASYNC_H

#include <functional>

#include "thread_pool.h"

template <class T>
class Async {
 public:
  static Future<T> async(std::function<T()> func, bool mode) {
    if (mode && pool.num_free_threads_.load() > 0) {
      return pool.Submit(func);
    } else {
      Promise<T> promise;
      try{
        promise.SetValue(func());
      } catch(const std::exception &excep) {
        promise.SetException(excep);
      }
      return promise.getFuture();
    }
  }
 private:
  static ThreadPool<T> pool;
};

#endif //PROMISE_ASYNC_H
