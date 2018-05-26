#ifndef PROMISE_FUTURE_PROMISE_H
#define PROMISE_FUTURE_PROMISE_H

#include <mutex>
#include <condition_variable>
#include <memory>

template <class T>
class Data {
 public:
  Data() = default;
 private:
  template <class U>
  friend class Future;
  template <class U>
  friend class Promise;
  T value;
  std::exception except;
  std::mutex mtx;
  std::condition_variable cond_var;
  int state = 0;  //  0=wait, 1=value, 2=exc
};

template <class T>
class Future {
 private:
  template <class U>
  friend class Promise;
  std::shared_ptr<Data<T>> data = nullptr;
  explicit Future(std::shared_ptr<Data<T>> data_) : data(data_) {}
 public:
  Future() = default;
  T Get() const {
    std::unique_lock<std::mutex> locker(data->mtx);
    data->cond_var.wait(locker, [&]{return data->state != 0;});
    if (data->state == 1) {
      return data->value;
    } else if (data->state == 2) {
      throw data->except;
    }
  }

  bool TryGet(T &val) const {
    std::unique_lock<std::mutex> locker(data->mtx);
    if (data->state == 1) {
      val = data->value;
      return true;
    } else if (data->state == 2) {
      throw data->except;
    } else {
      return false;
    }
  }
};

template <class T>
class Promise {
 private:
  std::shared_ptr<Data<T>> data = std::shared_ptr<Data<T>>(new Data<T>());
 public:
  Promise() = default;

  void SetValue(const T &val) {
    std::unique_lock<std::mutex> locker(data->mtx);
    data->state = 1;
    data->value = val;
    data->cond_var.notify_all();
  }

  Future<T> getFuture() const {
    return Future<T>(data);
  }

  void SetException(const std::exception &ex) {
    std::unique_lock<std::mutex> locker(data->mtx);
    data->except = ex;
    data->state = 2;
    data->cond_var.notify_all();
  }
};

#endif //PROMISE_FUTURE_PROMISE_H
