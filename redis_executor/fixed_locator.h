#ifndef _REDIS_FIXED_LOCATOR_H_
#define _REDIS_FIXED_LOCATOR_H_

#include <string>
#include <hiredis/hiredis.h>

#include "redis_locator.h"

namespace redis {

class FixedLocator : public RedisLocator {
 public:
  FixedLocator(const std::string & location) : location_(location) {
  }
  virtual ~FixedLocator(){}

  virtual std::string Locate(const char *);
 private:
  std::string location_;
};

}

#endif // _REDIS_FIXED_LOCATOR_H_

