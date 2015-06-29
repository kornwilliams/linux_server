#include "fixed_service_locator.h"

namespace melon {
namespace client {

std::string FixedServiceLocator::Locate(const char *) {
  return location_;
}

}
}

