#pragma once

#include <cassert>
#include "future_promise.h"
#include "async.h"

template<>
ThreadPool<int> Async<int>::pool{};

int foo() {
  return 5;
}

void test1() {
  Promise<int> my_promise;
  my_promise.SetValue(foo());
  Future<int> my_future = my_promise.getFuture();
  int future_value = my_future.Get();
  assert(future_value==foo());
}

void test2() {
  Promise<int> my_promise;
  my_promise.SetValue(foo());
  Future<int> my_future = my_promise.getFuture();
  int getter_2;
  my_future.TryGet(getter_2);
  assert(getter_2 == foo());
}

void test3() {
  Future<int> fut_2 = Async<int>::async(foo, false);
  int f_res_2 = fut_2.Get();
  assert(f_res_2 == foo());
}

void test4() {
  Future<int> fut_2 = Async<int>::async(foo, true);
  int f_res_2 = fut_2.Get();
  assert(f_res_2 == foo());
}

void test5() {
  Future<int> fut_2 = Async<int>::async(foo, false);
  int getter_2;
  fut_2.TryGet(getter_2);
  assert(getter_2 == foo());
}

void test6() {
  Future<int> fut_2 = Async<int>::async(foo, true);
  int getter_2;
  fut_2.TryGet(getter_2);
  assert(getter_2 == foo());
}

void testing() {
  test1();
  test2();
  test3();
  test4();
  test5();
  test6();
}
