#ifndef _LHT_USER_PROFILE_LOGIN_REQUEST_
#define _LHT_USER_PROFILE_LOGIN_REQUEST_

#include "base_request.h"

using ::fcgi::Request; 

namespace lht {
namespace fcgi {

class LoginRequest : public BaseRequest {
public:
  LoginRequest(FCGX_Request* r) : BaseRequest(r) {}
  virtual bool Response(Value& responseValue, Document::AllocatorType& allocator);
}; 

} // namespace fcgi

} // namespace lht

#endif // _LHT_USER_PROFILE_LOGIN_REQUEST_
