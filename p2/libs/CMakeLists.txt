cmake_minimum_required(VERSION 3.15)
project(Mythread)

set(CMAKE_CXX_STANDARD 14)

add_executable(Mythread uthread_yield.c uthread.h uthread.c private.h context.c)
#add_executable(Mythread contexts_fixed_schedule.c)