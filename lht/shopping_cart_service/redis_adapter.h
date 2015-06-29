#ifndef _LHT_SHOPPING_CART_REDIS_ADAPTER_
#define _LHT_SHOPPING_CART_REDIS_ADAPTER_

#include <boost/shared_ptr.hpp>
#include <set>
#include <map>
#include "redis_executor/redis_executor.h"

using std::set;
using std::map;
using boost::shared_ptr;

namespace lht {

class ShoppingCartRedisAdapter {
 public:
  ShoppingCartRedisAdapter();
  static ShoppingCartRedisAdapter& Instance();

  int32_t AddItems(const int64_t userId, const map<int64_t, int32_t>& itemMap);
  int32_t DelItems(const int64_t userId, const set<int64_t>& itemSet);
  int32_t UpdateItems(const int64_t userId, const map<int64_t, int32_t>& itemMap);
  int32_t GetItems(map<int64_t, int32_t>& itemMap, const int64_t userId);

 private:
  shared_ptr<redis::RedisExecutor> redis_exec_;
  
  int32_t _Del(const int64_t userId, const set<int64_t>& itemSet);
  int32_t _Set(const int64_t userId, const map<int64_t, int32_t>& itemMap);
  int32_t _Incr(const int64_t userId, const map<int64_t, int32_t>& itemMap);
};

}

#endif // _LHT_SHOPPING_CART_REDIS_ADAPTER_
