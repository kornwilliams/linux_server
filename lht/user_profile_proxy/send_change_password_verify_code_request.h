#ifndef _LHT_USER_PROFILE_SEND_CHANGE_PASSWORD_VERIFY_CODE_REQUEST_
#define _LHT_USER_PROFILE_SEND_CHANGE_PASSWORD_VERIFY_CODE_REQUEST_

#include "base_request.h"

using ::fcgi::Request; 

namespace lht {
namespace fcgi {

class SendChangePasswordVerifyCodeRequest : public BaseRequest {
public:
  SendChangePasswordVerifyCodeRequest(FCGX_Request* r) : BaseRequest(r) {}
  virtual bool Response(Value& responseValue, Document::AllocatorType& allocator);
}; 

} // namespace fcgi

} // namespace lht

#endif // _LHT_USER_PROFILE_SEND_CHANGE_PASSWORD_VERIFY_CODE_REQUEST_
