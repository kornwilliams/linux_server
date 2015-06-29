#ifndef _MELON_SERVICE_LOCATOR_H_
#define _MELON_SERVICE_LOCATOR_H_

#include <string>
#include <hiredis/hiredis.h>

namespace melon {
namespace client {

class ServiceLocator {
 public:
  virtual ~ServiceLocator(){}
  virtual std::string Locate(const char * server_key) = 0;
};

}
}

#endif // _MELON_SERVICE_LOCATOR_H_

