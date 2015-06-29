#ifndef _LHT_BINGO_REDIS_ADAPTER_
#define _LHT_BINGO_REDIS_ADAPTER_

#include <boost/shared_ptr.hpp>
#include <set>
#include "redis_executor/redis_executor.h"

using std::set;
using boost::shared_ptr;

namespace lht {

class BingoRedisAdapter {
 public:
  BingoRedisAdapter();
  static BingoRedisAdapter& Instance();

  int64_t UserEatAtRestaurant(const int64_t userId, const int64_t restaurantId);
  int64_t GetRestaurantListForUser(set<int64_t>& resList, const int64_t userId);

 private:
  shared_ptr<redis::RedisExecutor> redis_exec_;  
};

}

#endif // _LHT_BINGO_REDIS_ADAPTER_
