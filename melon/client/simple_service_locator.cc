#include "simple_service_locator.h"

namespace melon {
namespace client {

std::string SimpleServiceLocator::Locate(const char *) {
  return location_;
}

}
}

