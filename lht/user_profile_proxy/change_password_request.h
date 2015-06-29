#ifndef _LHT_USER_PROFILE_CHANGE_PASSWORD_REQUEST_
#define _LHT_USER_PROFILE_CHANGE_PASSWORD_REQUEST_

#include "base_request.h"

using ::fcgi::Request; 

namespace lht {
namespace fcgi {

class ChangePasswordRequest : public BaseRequest {
public:
  ChangePasswordRequest(FCGX_Request* r) : BaseRequest(r) {}
  virtual bool Response(Value& responseValue, Document::AllocatorType& allocator);
}; 

} // namespace fcgi

} // namespace lht

#endif // _LHT_USER_PROFILE_CHANGE_PASSWORD_REQUEST_
