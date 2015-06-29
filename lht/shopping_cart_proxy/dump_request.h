#ifndef _LHT_SHOPPING_CART_DUMP_REQUEST_
#define _LHT_SHOPPING_CART_DUMP_REQUEST_

#include "fcgi_service/fcgi_service.h"

using ::fcgi::Request; 

namespace lht {
namespace fcgi {

class DumpRequest : public Request {
public:
  DumpRequest(FCGX_Request* r) : Request(r) {}
  virtual bool Response();
};

} // namespace fcgi
} // namespace lht

#endif // _LHT_SHOPPING_CART_DUMP_REQUEST_

