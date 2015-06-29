#ifndef _LHT_USER_EAT_AT_RESTAURANT_TASK_
#define _LHT_USER_EAT_AT_RESTAURANT_TASK_

#include "thread_manager.h"

using namespace std;

namespace lht {

using apache::thrift::concurrency::Runnable;

class UserEatAtRestaurantTask : public Runnable {
 public:
  UserEatAtRestaurantTask(const int64_t uid, const int64_t rid);
  virtual void run();
 private:
  const int64_t userId; 
  const int64_t restaurantId;
};

}

#endif // _LHT_USER_EAT_AT_RESTAURANT_TASK_
