/*
 * yuwei.mu@gmail.com
 * 2013.5
 */
#ifndef _BASE_STOP_WATCH_H_
#define _BASE_STOP_WATCH_H_

#include <iostream>
#include <time.h>
 
namespace base {

// 秒表类, 用于代码块/函数调用等计时
class StopWatch {
 public:
  StopWatch(clockid_t which_clock = CLOCK_REALTIME) : which_clock_(which_clock) {
    clock_gettime(which_clock_, &start_);
  }

  void Reset() {
    clock_gettime(which_clock_, &start_);
  }

  // in nano-seconds
  timespec Elapsed() const;

 private:
  clockid_t which_clock_;
  timespec start_;

  StopWatch(const StopWatch&);
  StopWatch& operator= (const StopWatch&);
};

std::ostream & operator<<(std::ostream & os, const StopWatch& td);

}
 
#endif // _BASE_STOP_WATCH_H_

