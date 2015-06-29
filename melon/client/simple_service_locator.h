#ifndef _MELON_CLIENT_SIMPLE_SERVICE_LOCATOR_
#define _MELON_CLIENT_SIMPLE_SERVICE_LOCATOR_

#include <string>

#include "service_locator.h"

namespace melon {
namespace client {

class SimpleServiceLocator : public ServiceLocator {
 public:
  SimpleServiceLocator(const std::string & location): location_(location) {
  }
  virtual ~SimpleServiceLocator(){}

  virtual std::string Locate(const char *);
 private:
  std::string location_;
};

}
}

#endif // _MELON_CLIENT_SIMPLE_SERVICE_LOCATOR_

