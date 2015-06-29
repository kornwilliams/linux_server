#include "redis_executor.h"

#include <iostream>

#include "redis_executor/redis_context_pool.h"
#include "redis_executor/redis_locator.h"

namespace redis {

RedisExecutor::RedisExecutor(RedisLocator * locator) : locator_(locator), pool_(new RedisContextPool()) {
}

RedisExecutor::~RedisExecutor() {
  delete locator_;
  delete pool_;
}

redisReply * RedisExecutor::Execute(const char * server_key, const char * cmd, ...) {
  std::string endpoint;

  if (strlen(server_key) > 0) {
    endpoint = locator_->Locate(server_key);
  } else {
    endpoint = locator_->Locate(cmd);
  }

  redisContext * context = pool_->Alloc(endpoint);
  if (context == NULL) {
    LOG_WARN("RedisExecutor::Execute connect err, endpoint=" << endpoint);
    return NULL;
  }

  va_list ap;
  va_start(ap, cmd);
  redisReply * reply = (redisReply *)redisvCommand(context, cmd, ap);
  va_end(ap);

  LOG_DEBUG("RedisExecutor::Execute success=" << (reply != NULL));
  // TODO: context是否回收，应依据context，还是reply呢?
  pool_->Release(endpoint, context, reply != NULL);
  return reply;
}

redisContext * RedisExecutor::AllocContext(const char * server_key) {
  std::string endpoint = locator_->Locate(server_key);
  return pool_->Alloc(endpoint);
}

void RedisExecutor::ReleaseContext(const char *, redisContext * context, bool success) {
  pool_->Release(context, success);
}

void RedisExecutor::ReleaseContext(redisContext * context, bool success) {
  pool_->Release(context, success);
}

}

