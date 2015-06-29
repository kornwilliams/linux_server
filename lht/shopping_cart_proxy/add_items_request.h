#ifndef _LHT_SHOPPING_CART_ADD_ITEMS_REQUEST_
#define _LHT_SHOPPING_CART_ADD_ITEMS_REQUEST_

#include "base_request.h"

using ::fcgi::Request; 

namespace lht {
namespace fcgi {

class AddItemsRequest : public BaseRequest {
public:
  AddItemsRequest(FCGX_Request* r) : BaseRequest(r) {}
  virtual bool Response(Value& responseValue, Document::AllocatorType& allocator);
}; 

} // namespace fcgi

} // namespace lht

#endif // _LHT_SHOPPING_CART_ADD_ITEMS_REQUEST_
