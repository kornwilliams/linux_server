#ifndef _LHT_USER_PROFILE_REGISTER_REQUEST_
#define _LHT_USER_PROFILE_REGISTER_REQUEST_

#include "base_request.h"

using ::fcgi::Request; 

namespace lht {
namespace fcgi {

class RegisterRequest : public BaseRequest {
public:
  RegisterRequest(FCGX_Request* r) : BaseRequest(r) {}
  virtual bool Response(Value& responseValue, Document::AllocatorType& allocator);
}; 

} // namespace fcgi

} // namespace lht

#endif // _LHT_USER_PROFILE_REGISTER_REQUEST_
