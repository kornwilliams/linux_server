#ifndef _PE_PUSH_THREAD_MANAGER_
#define _PE_PUSH_THREAD_MANAGER_

#include <pthread.h>
#include <boost/shared_ptr.hpp>
#include <thrift/concurrency/ThreadManager.h>
#include <thrift/concurrency/TimerManager.h>

using apache::thrift::concurrency::ThreadManager;
using apache::thrift::concurrency::TimerManager;

boost::shared_ptr<ThreadManager> GetThreadManager();
boost::shared_ptr<TimerManager> GetTimerManager();

#endif // _PE_PUSH_THREAD_MANAGER_

