#include "stop_watch.h"
 
namespace base {

// in nano-seconds
timespec StopWatch::Elapsed() const {
  timespec now, elapsed;
  clock_gettime(which_clock_, &now);
  
  if (now.tv_nsec < start_.tv_nsec) {
    elapsed.tv_sec = now.tv_sec - start_.tv_sec - 1;
    elapsed.tv_nsec = 1000000000 + now.tv_nsec - start_.tv_nsec;
  } else {
    elapsed.tv_sec = now.tv_sec - start_.tv_sec;
    elapsed.tv_nsec = now.tv_nsec - start_.tv_nsec;
  }

  return elapsed;
}

std::ostream & operator<<(std::ostream & os, const StopWatch& td) {
  timespec el = td.Elapsed();
  return os << (el.tv_sec * 1000 + el.tv_nsec / 1000000) << "ms";
}

}
 

