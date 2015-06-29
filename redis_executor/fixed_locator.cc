#include "fixed_locator.h"

namespace redis {

std::string FixedLocator::Locate(const char *) {
  return location_;
}

}

