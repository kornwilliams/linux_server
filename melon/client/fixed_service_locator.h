#ifndef _MELON_CLIENT_FIXED_SERVICE_LOCATOR_
#define _MELON_CLIENT_FIXED_SERVICE_LOCATOR_

#include <string>

#include "service_locator.h"

namespace melon {
namespace client {

class FixedServiceLocator : public ServiceLocator {
 public:
  FixedServiceLocator(const std::string & location): location_(location) {
  }
  virtual ~FixedServiceLocator(){}

  virtual std::string Locate(const char *);
 private:
  std::string location_;
};

}
}

#endif // _MELON_CLIENT_FIXED_SERVICE_LOCATOR_

