#ifndef _LHT_USER_PROFILE_GET_USER_PROFILE_REQUEST_
#define _LHT_USER_PROFILE_GET_USER_PROFILE_REQUEST_

#include "base_request.h"

using ::fcgi::Request; 

namespace lht {
namespace fcgi {

class GetUserProfileRequest : public BaseRequest {
public:
  GetUserProfileRequest(FCGX_Request* r) : BaseRequest(r) {}
  virtual bool Response(Value& responseValue, Document::AllocatorType& allocator);

private:
  int TransformUserProfileToJson(Value& responseValue, Document::AllocatorType& allocator, UserProfile profile);
}; 

} // namespace fcgi

} // namespace lht

#endif // _LHT_USER_PROFILE_GET_USER_PROFILE_REQUEST_
