#include "base_service_impl.h"

#include <iostream>

namespace melon {
namespace service {

void BaseServiceImpl::Stop() {
  // service_->Stop();
  std::cout << "Stop() called, to stop service..." << std::endl;
}

}
}

