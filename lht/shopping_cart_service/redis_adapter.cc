#include <stdio.h>
#include "base/logging.h"
#include "base/config_reader.h"
#include "redis_executor/scoped_reply.h"
#include "redis_executor/replication_locator.h"
#include "redis_adapter.h"

using namespace redis;

namespace lht{

ShoppingCartRedisAdapter::ShoppingCartRedisAdapter() {
  ConfigReader cfg("../conf/lht_shopping_cart_service.conf");
  LOG_INFO("ShoppingCartRedisAdapter init : master=" << cfg.Get("redis", "master") << " slaves=" << cfg.Get("redis", "slaves"));
  redis_exec_= shared_ptr<RedisExecutor>(new RedisExecutor(new ReplicationLocator(cfg.Get("redis", "master"), cfg.Get("redis", "slaves"))));
}

ShoppingCartRedisAdapter& ShoppingCartRedisAdapter::Instance() {
  static ShoppingCartRedisAdapter instance;
  return instance;
}

int32_t ShoppingCartRedisAdapter::_Del(const int64_t userId, const set<int64_t>& itemSet) {
  const int MAX_BUF_LEN = 512;
  char cmd_buf[MAX_BUF_LEN];
  cmd_buf[MAX_BUF_LEN - 1] = '\0';
  int offset = 0, ret = 0;
 
  // TODO 检查ret和offset是否为负 
  offset = snprintf(cmd_buf, MAX_BUF_LEN - 1, "hdel lht:shoppingcart:%ld", userId);
  for (set<int64_t>::const_iterator it = itemSet.begin(); ret >= 0 && it != itemSet.end(); ++it) {
    ret = snprintf(cmd_buf + offset, MAX_BUF_LEN - 1 - offset, " %ld", *it);
    offset += ret;
  }

  ret = 0;
  for(int i = 0; i < 3; ++i) {
    ScopedReply reply(redis_exec_->Execute("/w", cmd_buf));
    if (!reply) {
      ret = -1;
    } else if (reply->type != REDIS_REPLY_INTEGER) {
      ret = -2;
    } else {
      ret = reply->integer;  // 正常执行时ret应当等于0
      break;
    }
  }

  if (ret < 0) {
    LOG_WARN("ShoppingCartRedisAdapter _Del err : userId=" << userId << " itemSet.size=" << itemSet.size() << " ret=" << ret << " result=命令执行出错" << " cmd_buf=" << cmd_buf);
  } else {
    LOG_DEBUG("ShoppingCartRedisAdapter _Del ok : userId=" << userId << " itemSet.size=" << itemSet.size() << " ret=" << ret);
  }
  return ret;
}

int32_t ShoppingCartRedisAdapter::_Set(const int64_t userId, const map<int64_t, int32_t>& itemMap) {
  const int MAX_BUF_LEN = 512;
  char cmd_buf[MAX_BUF_LEN];
  cmd_buf[MAX_BUF_LEN - 1] = '\0';
  int offset = 0, ret = 0;

  offset = snprintf(cmd_buf, MAX_BUF_LEN - 1, "hmset lht:shoppingcart:%ld", userId);
  for (map<int64_t, int32_t>::const_iterator it = itemMap.begin(); ret >= 0 && it != itemMap.end(); ++it) {
    ret = snprintf(cmd_buf + offset, MAX_BUF_LEN - 1 - offset, " %ld %d", it->first, it->second);
    offset += ret;
  }

  ret = 0;
  for(int i = 0; i < 3; ++i) {
    ScopedReply reply(redis_exec_->Execute("/w", cmd_buf));
    if (!reply) {
      ret = -1;
    } else if (reply->type != REDIS_REPLY_STATUS || strcasecmp(reply->str, "OK") != 0) {
      ret = -2;
    } else {
      ret = 0;
      break;
    }
  }

  if (ret < 0) {
    LOG_WARN("ShoppingCartRedisAdapter _Set err : userId=" << userId << " itemMap.size=" << itemMap.size() << " ret=" << ret << " result=命令执行出错" << " cmd_buf=" << cmd_buf);
  } else {
    LOG_DEBUG("ShoppingCartRedisAdapter _Set ok : userId=" << userId << " itemMap.size=" << itemMap.size() << " ret=" << ret);
  }
  return ret;
}

// TODO
// redis的hincrby命令无法一次设置多个field
// 暂定使用循环遍历map执行多条命令的方式执行
// 之后应当编辑库文件，增加对pipeline的支持，然后是使用pipeline方式替代多命令方式
int32_t ShoppingCartRedisAdapter::_Incr(const int64_t userId, const map<int64_t, int32_t>& itemMap) {
  const int MAX_BUF_LEN = 256;
  char cmd_buf[MAX_BUF_LEN];
  cmd_buf[MAX_BUF_LEN - 1] = '\0';
  int offset = 0, offsetField = 0, ret = 0;

  snprintf(cmd_buf, MAX_BUF_LEN - 1, "hincrby lht:shoppingcart:%ld", userId);
  
  for (map<int64_t, int32_t>::const_iterator it = itemMap.begin(); it != itemMap.end(); ++it) {
    offsetField = snprintf(cmd_buf + offset, MAX_BUF_LEN - 1 - offset, " %ld %d", it->first, it->second);
    ret = 0;
    for(int i = 0; i < 3; ++i) {
      ScopedReply reply(redis_exec_->Execute("/w", cmd_buf));
      if (!reply) {
        ret = -1;
      } else if (reply->type != REDIS_REPLY_INTEGER) {
        ret = -2;
      } else {
        ret = reply->integer;
        break;
      }
    }

    if (ret < 0) {
      LOG_WARN("ShoppingCartRedisAdapter _Incr err : userId=" << userId << " productId=" << it->first << " num=" << it->second << " ret=" << ret << " result=命令执行出错" << " cmd_buf=" << cmd_buf);
      break;
    } else {
      LOG_DEBUG("ShoppingCartRedisAdapter _Incr processing : userId=" << userId << " productId=" << it->first << " num=" << it->second << " ret=" << ret);
    }
  }

  return (ret < 0 ? -1 : 0);
}

int32_t ShoppingCartRedisAdapter::AddItems(const int64_t userId, const map<int64_t, int32_t>& itemMap) {
  return _Incr(userId, itemMap);
}

int32_t ShoppingCartRedisAdapter::DelItems(const int64_t userId, const set<int64_t>& itemSet) {
  return _Del(userId, itemSet);
}

int32_t ShoppingCartRedisAdapter::UpdateItems(const int64_t userId, const map<int64_t, int32_t>& itemMap) {
  map<int64_t, int32_t> setMap;
  set<int64_t> delSet;
  int32_t ret1 = 0, ret2 = 0;

  // 其实可以不使用setMap，只需要在itemMap里面把delSet中的元素去掉
  // itemMap剩下的部分就等于setMap
  // 但为了保持itemMap的const性质，暂不采取此思路
  for (map<int64_t, int32_t>::const_iterator it = itemMap.begin(); it != itemMap.end(); ++it) {
    if (it->second <= 0)
      delSet.insert(it->first);
    else
      setMap[it->first] = it->second;
  }

  if (setMap.size() > 0) {
    ret1 = _Set(userId, setMap);
    if (ret1 < 0) {
      LOG_WARN("ShoppingCartRedisAdapter UpdateItems err : userId=" << userId << " itemMap.size=" << itemMap.size() << " ret=-1 ret1=" << ret1 << " result=执行插入部分时_Set调用出错");
      return -1;
    }
  }

  if (delSet.size() > 0) {
    ret2 = _Del(userId, delSet);
    if (ret2 < 0) {
      LOG_WARN("ShoppingCartRedisAdapter UpdateItems err : userId=" << userId << " itemMap.size=" << itemMap.size() << " ret=-2 ret2=" << ret2 << " result=执行删除部分时_Del调用出错");
      return -2;
    }
  }

  LOG_DEBUG("ShoppingCartRedisAdapter UpdateItems ok : userId=" << userId << " itemMap.size=" << itemMap.size() << " ret1=" << ret1 << " ret2=" << ret2);
  return ret1 + ret2;
}

int32_t ShoppingCartRedisAdapter::GetItems(map<int64_t, int32_t>& itemMap, const int64_t userId) {
  const int MAX_BUF_LEN = 256;
  char cmd_buf[MAX_BUF_LEN];
  cmd_buf[MAX_BUF_LEN - 1] = '\0';

  snprintf(cmd_buf, MAX_BUF_LEN - 1, "hgetall lht:shoppingcart:%ld", userId);

  int ret = 0;
  for(int i = 0; i < 3; ++i) {
    ScopedReply reply(redis_exec_->Execute("/r", cmd_buf));
    if (!reply) {
      ret = -1;
    } else if (reply->type != REDIS_REPLY_ARRAY || (reply->elements & 0x1) == 1) {
      ret = -2;
    } else {
      itemMap.clear();
      for (size_t i = 0; i < reply->elements; i += 2) {
        itemMap[atol(reply->element[i]->str)] = atoi(reply->element[i + 1]->str);
      }
      ret = reply->elements / 2;
    }
  }

  if (ret < 0) {
    LOG_WARN("ShoppingCartRedisAdapter GetItems err : userId=" << userId << " ret=" << ret << " cmd_buf=" << cmd_buf);
  } else {
    LOG_DEBUG("ShoppingCartRedisAdapter GetItems ok : userId=" << userId << " ret=" << ret);
  }
  return ret;
}

} // namespace lht
