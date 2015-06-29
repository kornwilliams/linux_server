#include "BaseService.h"

namespace melon {
namespace service {

class BaseServiceImpl : virtual public BaseServiceIf {
 public:
  virtual void Stop();
};

}
}

