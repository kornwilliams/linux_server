#include <stdio.h>
#include <sstream>
#include "base/logging.h"
#include "base/config_reader.h"
#include "redis_executor/scoped_reply.h"
#include "redis_executor/replication_locator.h"
#include "redis_adapter.h"

using namespace redis;
using namespace std;

namespace lht{

UserProfileRedisAdapter::UserProfileRedisAdapter() {
  ConfigReader cfg("../conf/lht_shopping_cart_service.conf");
  EXPIRE_TIME = cfg.GetWithType<int>("redis", "expire", 600);
  LOG_INFO("UserProfileRedisAdapter init : master=" << cfg.Get("redis", "master") << " slaves=" << cfg.Get("redis", "slaves") << " expire=" << EXPIRE_TIME);
  redis_exec_= shared_ptr<RedisExecutor>(new RedisExecutor(new ReplicationLocator(cfg.Get("redis", "master"), cfg.Get("redis", "slaves"))));
}

UserProfileRedisAdapter& UserProfileRedisAdapter::Instance() {
  static UserProfileRedisAdapter instance;
  return instance;
}

int UserProfileRedisAdapter::_Set(const string& key, const int32_t code) {
  const int MAX_BUF_LEN = 512;
  char cmd_buf[MAX_BUF_LEN];
  cmd_buf[MAX_BUF_LEN - 1] = '\0';
  snprintf(cmd_buf, MAX_BUF_LEN - 1, "SET %s %d", key.c_str(), code);

  int ret = 0;
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
    LOG_WARN("UserProfileRedisAdapter _Set err : ret=" << ret << " cmd_buf=" << cmd_buf);
  } else {
    LOG_DEBUG("UserProfileRedisAdapter _Set ok : ret=" << ret << " cmd_buf=" << cmd_buf);
  }
  return ret;
}

int UserProfileRedisAdapter::_Expire(const string& key, const int time) {
  const int MAX_BUF_LEN = 512;
  char cmd_buf[MAX_BUF_LEN];
  cmd_buf[MAX_BUF_LEN - 1] = '\0';
  snprintf(cmd_buf, MAX_BUF_LEN - 1, "EXPIRE %s %d", key.c_str(), time);

  int ret = 0;
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
    LOG_WARN("UserProfileRedisAdapter _Expire err : ret=" << ret << " cmd_buf=" << cmd_buf);
  } else {
    LOG_DEBUG("UserProfileRedisAdapter _Expire ok : ret=" << ret << " cmd_buf=" << cmd_buf);
  }
  return ret;
}

int UserProfileRedisAdapter::_Get(const string& key) {
  const int MAX_BUF_LEN = 512;
  char cmd_buf[MAX_BUF_LEN];
  cmd_buf[MAX_BUF_LEN - 1] = '\0';
  snprintf(cmd_buf, MAX_BUF_LEN - 1, "GET %s", key.c_str());

  int ret = 0;
  for(int i = 0; i < 3; ++i) {
    ScopedReply reply(redis_exec_->Execute("/r", cmd_buf));
    if (!reply) {
      ret = -1;
    } else if (reply->type != REDIS_REPLY_STRING) {
      ret = -2;
    } else {
      ret = atoi(reply->str);
      break;
    }
  }

  if (ret < 0) {
    LOG_WARN("UserProfileRedisAdapter _Get err : ret=" << ret << " cmd_buf=" << cmd_buf);
  } else {
    LOG_DEBUG("UserProfileRedisAdapter _Get ok : ret=" << ret << " cmd_buf=" << cmd_buf);
  }
  return ret;
}

int UserProfileRedisAdapter::_Del(const string& key) {
  const int MAX_BUF_LEN = 512;
  char cmd_buf[MAX_BUF_LEN];
  cmd_buf[MAX_BUF_LEN - 1] = '\0';
  snprintf(cmd_buf, MAX_BUF_LEN - 1, "DEL %s", key.c_str());

  int ret = 0;
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
    LOG_WARN("UserProfileRedisAdapter _Del err : ret=" << ret << " cmd_buf=" << cmd_buf);
  } else {
    LOG_DEBUG("UserProfileRedisAdapter _Del ok : ret=" << ret << " cmd_buf=" << cmd_buf);
  }
  return ret;
}

int32_t UserProfileRedisAdapter::SetVerifyCode(const string& prefix, const string& phone, const int32_t code) {
  stringstream key;
  key << "lht:" << prefix << ":" << phone;

  if (_Set(key.str(), code) < 0)
    return -1;
  
  if (_Expire(key.str(), EXPIRE_TIME) < 0) {
    return -2;
  }
  
  return 0;
}

int UserProfileRedisAdapter::GetVerifyCode(const string& prefix, const string& phone) {
  stringstream key;
  key << "lht:" << prefix << ":" << phone;
  return _Get(key.str());
}

int UserProfileRedisAdapter::DelVerifyCode(const string& prefix, const string& phone) {
  stringstream key;
  key << "lht:" << prefix << ":" << phone;
  return _Del(key.str());
}

int UserProfileRedisAdapter::AddUser(const string& phone, const int64_t uid) {
  const int MAX_BUF_LEN = 512;
  char cmd_buf[MAX_BUF_LEN];
  cmd_buf[MAX_BUF_LEN - 1] = '\0';
  snprintf(cmd_buf, MAX_BUF_LEN - 1, "HSET lht:user:uid:table %s %ld", phone.c_str(), uid); 

  int ret = 0;
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
    LOG_WARN("UserProfileRedisAdapter AddUser err : ret=" << ret << " cmd_buf=" << cmd_buf);
  } else {
    LOG_DEBUG("UserProfileRedisAdapter AddUser ok : ret=" << ret << " cmd_buf=" << cmd_buf);
  }
  return ret;
}

int UserProfileRedisAdapter::CheckUser(const string& phone) {
  const int MAX_BUF_LEN = 256;
  char cmd_buf[MAX_BUF_LEN];
  cmd_buf[MAX_BUF_LEN - 1] = '\0';
  snprintf(cmd_buf, MAX_BUF_LEN - 1, "HEXISTS lht:user:uid:table %s", phone.c_str());

  int ret = 0;
  for(int i = 0; i < 3; ++i) {
    ScopedReply reply(redis_exec_->Execute("/r", cmd_buf));
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
    LOG_WARN("UserProfileRedisAdapter CheckUser err : ret=" << ret << " cmd_buf=" << cmd_buf);
  } else {
    LOG_DEBUG("UserProfileRedisAdapter CheckUser ok : ret=" << ret << " cmd_buf=" << cmd_buf);
  }
  return ret;
}

int UserProfileRedisAdapter::GetUser(const string& phone, int64_t& uid) {
  const int MAX_BUF_LEN = 512;
  char cmd_buf[MAX_BUF_LEN];
  cmd_buf[MAX_BUF_LEN - 1] = '\0';
  snprintf(cmd_buf, MAX_BUF_LEN - 1, "HGET lht:user:uid:table %s", phone.c_str());

  int ret = 0;
  for(int i = 0; i < 3; ++i) {
    ScopedReply reply(redis_exec_->Execute("/r", cmd_buf));
    if (!reply) {
      ret = -1;
    } else if (reply->type != REDIS_REPLY_STRING) {
      ret = -2;
    } else {
      ret = 0;
      uid = atol(reply->str);
      break;
    }
  }

  if (ret < 0) {
    LOG_WARN("UserProfileRedisAdapter GetUser err : ret=" << ret << " cmd_buf=" << cmd_buf);
  } else {
    LOG_DEBUG("UserProfileRedisAdapter GetUser ok : ret=" << ret << " cmd_buf=" << cmd_buf);
  }
  return ret;
}

int UserProfileRedisAdapter::SetPassword(const string& phone, const string& pw) {
  const int MAX_BUF_LEN = 512;
  char cmd_buf[MAX_BUF_LEN];
  cmd_buf[MAX_BUF_LEN - 1] = '\0';
  snprintf(cmd_buf, MAX_BUF_LEN - 1, "HSET lht:user:password:table %s %s", phone.c_str(), pw.c_str());

  int ret = 0;
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
    LOG_WARN("UserProfileRedisAdapter SetPassword err : ret=" << ret << " cmd_buf=" << cmd_buf);
  } else {
    LOG_DEBUG("UserProfileRedisAdapter SetPassword ok : ret=" << ret << " cmd_buf=" << cmd_buf);
  }
  return ret;
}

int UserProfileRedisAdapter::GetPassword(const string& phone, string& pw) {
  const int MAX_BUF_LEN = 512;
  char cmd_buf[MAX_BUF_LEN];
  cmd_buf[MAX_BUF_LEN - 1] = '\0';
  snprintf(cmd_buf, MAX_BUF_LEN - 1, "HGET lht:user:password:table %s", phone.c_str());

  int ret = 0;
  for(int i = 0; i < 3; ++i) {
    ScopedReply reply(redis_exec_->Execute("/r", cmd_buf));
    if (!reply) {
      ret = -1;
    } else if (reply->type != REDIS_REPLY_STRING) {
      ret = -2;
    } else {
      ret = 0;
      pw = reply->str;
      break;
    }
  }

  if (ret < 0) {
    LOG_WARN("UserProfileRedisAdapter GetPassword err : ret=" << ret << " cmd_buf=" << cmd_buf);
  } else {
    LOG_DEBUG("UserProfileRedisAdapter GetPassword ok : ret=" << ret << " cmd_buf=" << cmd_buf);
  }
  return ret;
}

} // namespace lht
