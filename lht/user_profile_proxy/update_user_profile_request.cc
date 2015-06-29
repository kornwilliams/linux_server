#include "base/logging.h"
#include "update_user_profile_request.h"
#include "lht/adapters/lht_user_profile_service_adapter.h"

using namespace std;
using namespace rapidjson;

namespace lht {
namespace fcgi {

bool UpdateUserProfileRequest::Response(Value& ret, Document::AllocatorType& allocator) {
  const int64_t uid = UpdateQueryLong("userId");
  const string& key = UpdateQuery("key");

  char params[256];
  snprintf(params, 255, "userId=%ld key=%s", uid, key.c_str()); 

  LOG_INFO("UpdateUserProfileRequest receive : " << params); 

  if (VerifyRequest(key, uid) < 0) {
    LOG_WARN("UpdateUserProfileRequest err : " << params);
    ResponseWithJson(ret, allocator, 1000, "请求未通过验证");
    return true;
  }

  int status = UserProfileServiceAdapter::Instance().UpdateUserProfile(profile, 3);
  if (status == 2001) {
    LOG_WARN("UpdateUserProfileRequest err : " << params << " code=2001 msg=记录不存在");
    ResponseWithJson(ret, allocator, 2001, "记录不存在");
    return true;
  } else if (uid < 0) {
    LOG_WARN("UpdateUserProfileRequest err : " << params << " code=2000 msg=UserProfileService存在故障");
    ResponseWithJson(ret, allocator, 2000, "服务器故障");
    return true;
  }

  if (TransformUserProfileToJson(ret, allocator, profile) < 0) {
    LOG_WARN("UpdateUserProfileRequest err : " << params << " code=2000 msg=UserProfileService存在故障");
    ResponseWithJson(ret, allocator, 2000, "服务器故障");
    return true;
  }

  LOG_DEBUG("UpdateUserProfileRequest ok : " << params << uid << " code=0 msg=ok"); 
  ResponseWithJson(ret, allocator, 0, "ok");
  return true; 
} // Response

int UpdateUserProfileRequest::TransformUserProfileToJson(Value& ret, Document::AllocatorType& allocator, UserProfile& profile) {
  Value value(kObjectType);
  Value strValue;
  
  value.AddMember("userId", profile.userId, allocator);
  
  strValue.SetString(profile.phone, allocator);
  value.AddMember("phone", strValue, allocator);

  strValue.SetString(profile.nickname, allocator);
  value.AddMember("nickname", strValue, allocator);

  strValue.SetString(profile.headurl, allocator);
  value.AddMember("headurl", strValue, allocator);

  strValue.SetString(profile.birthday, allocator);
  value.AddMember("birthday", strValue, allocator);

  value.AddMember("gender", profile.gender, allocator);

  return 0;
}

} // namespace fcgi
} // namespace lht
