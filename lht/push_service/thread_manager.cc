#include "thread_manager.h"

#include <boost/shared_ptr.hpp>
#include <thrift/concurrency/PlatformThreadFactory.h>

using apache::thrift::concurrency::PlatformThreadFactory;

boost::shared_ptr<ThreadManager> GetThreadManager() {
  static boost::shared_ptr<ThreadManager> thread_manager;
  if (!thread_manager) {
    thread_manager = ThreadManager::newSimpleThreadManager(100, 50000);
    boost::shared_ptr<PlatformThreadFactory> factory = boost::shared_ptr<PlatformThreadFactory>(new PlatformThreadFactory());
    thread_manager->threadFactory(factory);
    thread_manager->start();
  }
  return thread_manager;
}

boost::shared_ptr<TimerManager> GetTimerManager() {
  static boost::shared_ptr<TimerManager> timer_manager;
  if (!timer_manager) {
    timer_manager.reset(new TimerManager());
    timer_manager->threadFactory(boost::shared_ptr<PlatformThreadFactory>(new PlatformThreadFactory()));
    timer_manager->start();
  }
  return timer_manager; 
}

