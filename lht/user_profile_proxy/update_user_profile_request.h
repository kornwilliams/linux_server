#ifndef _LHT_USER_PROFILE_UPDATE_USER_PROFILE_REQUEST_
#define _LHT_USER_PROFILE_UPDATE_USER_PROFILE_REQUEST_

#include "base_request.h"

using ::fcgi::Request; 

namespace lht {
namespace fcgi {

class UpdateUserProfileRequest : public BaseRequest {
public:
  UpdateUserProfileRequest(FCGX_Request* r) : BaseRequest(r) {}
  virtual bool Response(Value& responseValue, Document::AllocatorType& allocator);
}; 

} // namespace fcgi

} // namespace lht

#endif // _LHT_USER_PROFILE_UPDATE_USER_PROFILE_REQUEST_
