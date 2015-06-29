#ifndef _REDIS_REDIS_EXECUTOR_H_
#define _REDIS_REDIS_EXECUTOR_H_

#include <hiredis/hiredis.h>

namespace redis {

class RedisContextPool;
class RedisLocator;

class RedisExecutor {
 public:
  RedisExecutor(RedisLocator * locator);
  ~RedisExecutor();

  redisReply * Execute(const char * server_key, const char * cmd, ...);

  redisContext * AllocContext(const char * server_key);

  void ReleaseContext(const char * server_key, redisContext * context, bool success);
  void ReleaseContext(redisContext * context, bool success);

 private:
  RedisLocator * locator_;
  RedisContextPool * pool_;
};

}

#endif // _REDIS_REDIS_EXECUTOR_H_

