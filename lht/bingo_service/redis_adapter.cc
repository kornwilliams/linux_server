#include <stdio.h>
#include "base/logging.h"
#include "base/config_reader.h"
#include "redis_executor/scoped_reply.h"
#include "redis_executor/replication_locator.h"
#include "redis_adapter.h"

using namespace redis;

namespace lht{

BingoRedisAdapter::BingoRedisAdapter() {
  ConfigReader cfg("../conf/lht_bingo_service.conf");
  LOG_INFO("BingoRedisAdapter init : master=" << cfg.Get("redis", "master") << " slaves=" << cfg.Get("redis", "slaves"));
  redis_exec_= shared_ptr<RedisExecutor>(new RedisExecutor(new ReplicationLocator(cfg.Get("redis", "master"), cfg.Get("redis", "slaves"))));
}

BingoRedisAdapter& BingoRedisAdapter::Instance() {
  static BingoRedisAdapter instance;
  return instance;
}

int64_t BingoRedisAdapter::UserEatAtRestaurant(const int64_t userId, const int64_t restaurantId) {
  const int MAX_BUF_LEN = 256;
  char cmd_buf[MAX_BUF_LEN];
  cmd_buf[MAX_BUF_LEN - 1] = '\0';

  snprintf(cmd_buf, MAX_BUF_LEN - 1, "sadd lht:bingo:%lld %lld", userId, restaurantId);

  int ret = 0;
  for(int i = 0; i < 3; ++i) {
    ScopedReply reply(redis_exec_->Execute("/w", cmd_buf));
    
    if (!reply) {
      ret = -1;
    } else if (reply->type != REDIS_REPLY_INTEGER) {
      ret = -2;
    } else {
      ret = reply->integer; // 0 or 1
      break;
    }
  }

  if (ret < 0) {
    LOG_WARN("BingoRedisAdapter UserEatAtRestaurant err : userId=" << userId << " restaurantId=" << restaurantId << " ret=" << ret << " cmd_buf=" << cmd_buf);
  } else {
    LOG_DEBUG("BingoRedisAdapter UserEatAtRestaurant ok : userId=" << userId << " restaurantId=" << restaurantId << " ret=" << ret);
  }
  return ret;
}

int64_t BingoRedisAdapter::GetRestaurantListForUser(set<int64_t>& resList, const int64_t userId) {
  const int MAX_BUF_LEN = 256;
  char cmd_buf[MAX_BUF_LEN];
  cmd_buf[MAX_BUF_LEN - 1] = '\0';

  snprintf(cmd_buf, MAX_BUF_LEN - 1, "SMEMBERS lht:bingo:%lld", userId);

  int ret = 0;
  for(int i = 0; i < 3; ++i) {
    ScopedReply reply(redis_exec_->Execute("/r", cmd_buf));
    
    if (!reply) {
      ret = -1;
    } else if (reply->type == REDIS_REPLY_ARRAY) {
      ret = -2;
    } else {
      resList.clear();
      for (size_t i = 0; i < reply->elements; ++i) {
        resList.insert(atol(reply->element[i]->str));
      }
      ret = 0;
      break;
    }
  }

  if (ret < 0) {
    LOG_WARN("BingoRedisAdapter GetRestaurantListForUser err : userId=" << userId << " size=" << resList.size() << " ret=" << ret << " cmd_buf=" << cmd_buf);
  } else {
    LOG_DEBUG("BingoRedisAdapter GetRestaurantListForUser ok : userId=" << userId << " size=" << resList.size() << " ret=" << ret);
  }
  return ret;
}

} // namespace lht
