#ifndef _REDIS_REDIS_CONTEXT_POOL_H_
#define _REDIS_REDIS_CONTEXT_POOL_H_

#include <hiredis/hiredis.h>

#include "base/object_pool.h"

namespace redis {

class RedisContextPool : public base::ObjectPool<redisContext> {
 public:
  RedisContextPool() : base::ObjectPool<redisContext>(0, 20, 0, 100) {
  }
  virtual ~RedisContextPool();
 protected:
  virtual redisContext * Create(const std::string & key);
  virtual void Destroy(redisContext * context);
};

}

#endif // _REDIS_REDIS_CONTEXT_POOL_H_

