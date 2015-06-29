#ifndef _LHT_USER_PROFILE_REDIS_ADAPTER_
#define _LHT_USER_PROFILE_REDIS_ADAPTER_
#include <string>
#include <boost/shared_ptr.hpp>
#include "redis_executor/redis_executor.h"

using std::string;
using boost::shared_ptr;

namespace lht {

class UserProfileRedisAdapter {
 public:
  UserProfileRedisAdapter();
  static UserProfileRedisAdapter& Instance();

  int SetVerifyCode(const string& prefix, const string& phone, const int32_t code);
  int GetVerifyCode(const string& prefix, const string& phone);
  int DelVerifyCode(const string& prefix, const string& phone);
  
  int AddUser(const string& phone, const int64_t uid);
  int CheckUser(const string& phone);
  int GetUser(const string& phone, int64_t& uid);

  int SetPassword(const string& phone, const string& password);
  int GetPassword(const string& phone, string& password);

 private:
  shared_ptr<redis::RedisExecutor> redis_exec_;
  int EXPIRE_TIME;

  int _Del(const string& key);
  int _Get(const string& key);
  int _Expire(const string& key, const int seconds);
  int _Set(const string& key, const int32_t code);
};

}

#endif // _LHT_USER_PROFILE_REDIS_ADAPTER_
