#ifndef _LHT_SHOPPING_CART_UPDATE_ITEMS_REQUEST_
#define _LHT_SHOPPING_CART_UPDATE_ITEMS_REQUEST_

#include "base_request.h"

using ::fcgi::Request; 

namespace lht {
namespace fcgi {

class UpdateItemsRequest : public BaseRequest {
public:
  UpdateItemsRequest(FCGX_Request* r) : BaseRequest(r) {}
  virtual bool Response(Value& responseValue, Document::AllocatorType& allocator);
}; 

} // namespace fcgi

} // namespace lht

#endif // _LHT_SHOPPING_CART_UPDATE_ITEMS_REQUEST_
