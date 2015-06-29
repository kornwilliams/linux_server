#ifndef _LHT_USER_PROFILE_SEND_REGISTER_VERIFY_CODE_REQUEST_
#define _LHT_USER_PROFILE_SEND_REGISTER_VERIFY_CODE_REQUEST_

#include "base_request.h"

using ::fcgi::Request; 

namespace lht {
namespace fcgi {

class SendRegisterVerifyCodeRequest : public BaseRequest {
public:
  SendRegisterVerifyCodeRequest(FCGX_Request* r) : BaseRequest(r) {}
  virtual bool Response(Value& responseValue, Document::AllocatorType& allocator);
}; 

} // namespace fcgi

} // namespace lht

#endif // _LHT_USER_PROFILE_SEND_REGISTER_VERIFY_CODE_REQUEST_
