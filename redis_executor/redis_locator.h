#ifndef _REDIS_REDIS_LOCATOR_H_
#define _REDIS_REDIS_LOCATOR_H_

#include <string>
#include <hiredis/hiredis.h>

namespace redis {

class RedisLocator {
 public:
  virtual ~RedisLocator(){}
  virtual std::string Locate(const char * server_key) = 0;
};

}

#endif // _REDIS_REDIS_LOCATOR_H_

