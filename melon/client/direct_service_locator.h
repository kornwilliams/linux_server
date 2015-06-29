#ifndef _MELON_CLIENT_DIRECT_SERVICE_LOCATOR_
#define _MELON_CLIENT_DIRECT_SERVICE_LOCATOR_

#include <string>

#include "service_locator.h"

namespace melon {
namespace client {

class DirectServiceLocator : public ServiceLocator {
 public:
  DirectServiceLocator() {}

  virtual ~DirectServiceLocator(){}

  virtual std::string Locate(const char *);
};

}
}

#endif // _MELON_CLIENT_DIRECT_SERVICE_LOCATOR_

