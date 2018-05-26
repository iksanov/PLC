cmake_minimum_required(VERSION 3.8)
project(promise)

set(CMAKE_CXX_STANDARD 14)

set(SOURCE_FILES main.cpp thread_pool.h future_promise.h blocking_queue.h async.h tests.h)
add_executable(promise ${SOURCE_FILES})