#include "direct_service_locator.h"

namespace melon {
namespace client {

std::string DirectServiceLocator::Locate(const char * server_key) {
  return server_key;
}

}
}

