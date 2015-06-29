#include "redis_context_pool.h"
#include "redis_locator.h"
#include "base/logging.h"

#include <boost/lexical_cast.hpp>

namespace redis {

static bool ParseAddr(const std::string & addr, std::string * host, int * port, std::string *password) {
  size_t pos = addr.find_first_of(':');
  if (pos == std::string::npos) {
    return false;
  }
  size_t pos2 = addr.find_first_of(':', pos + 1);
  *host = addr.substr(0, pos);

  try {
    if (pos2 == std::string::npos) {
      LOG_DEBUG("redis host=" << *host << " port=" << addr.substr(pos + 1) << " without pwd");
      *port = boost::lexical_cast<short>(addr.substr(pos + 1));
    } else {
      *port = boost::lexical_cast<short>(addr.substr(pos + 1, pos2 - pos - 1));
      *password = addr.substr(pos2 + 1);
      LOG_DEBUG("redis host=" << *host << " port=" << addr.substr(pos + 1, pos2 - pos - 1) << " pwd_len=" << password->size());
    }
  } catch (boost::bad_lexical_cast &) {
    return false;
  }
  return true;
}

RedisContextPool::~RedisContextPool() {
}

redisContext * RedisContextPool::Create(const std::string & endpoint) {
  const static struct timeval timeout = {0, 200000}; // 200ms连接超时
  LOG_INFO("Create redis client, endpoint=" << endpoint);

  std::string host, password;
  int port;

  if (!ParseAddr(endpoint, &host, &port, &password)) {
    LOG_WARN("bad redis service addr : " << endpoint);
    return NULL;
  }

  redisContext *context = redisConnectWithTimeout((char*)host.c_str(), port, timeout);
  if (context->err) {
    LOG_WARN("redis host=" << host << "port=" << port << " connect error: " << context->errstr);
    redisFree(context);
    return NULL;
  }

  if (!password.empty()) {
    redisReply *reply = (redisReply *)redisCommand(context, "AUTH %s", password.c_str());
    if (!(reply && reply->type == REDIS_REPLY_STATUS && strcasecmp(reply->str, "ok") == 0)) {
      LOG_WARN("redis auth error.");
      redisFree(context);
      return NULL;
    }
    freeReplyObject(reply);
  }

  LOG_INFO("new redis connection " << host << ":" << port << " established.");
  return context;
}

void RedisContextPool::Destroy(redisContext * context) {
  LOG_DEBUG("redis connection " << context << " destroyed. ");
  redisFree(context);
}

}


