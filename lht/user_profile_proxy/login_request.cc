#include "base/logging.h"
#include "login_request.h"
#include "lht/adapters/lht_user_profile_service_adapter.h"

using namespace std;
using namespace rapidjson;

namespace lht {
namespace fcgi {

bool LoginRequest::Response(Value& ret, Document::AllocatorType& allocator) {
  const string& phone = GetQuery("phone");
  const string& pw = GetQuery("password");
  const string& key = GetQuery("key");

  char params[256];
  snprintf(params, 255, "phone=%s pw=%s key=%s", phone.c_str(), pw.c_str(), key.c_str()); 

  LOG_INFO("LoginRequest receive : " << params); 

  if (VerifyRequest(key, phone.c_str(), pw.c_str()) < 0) {
    LOG_WARN("LoginRequest err : " << params);
    ResponseWithJson(ret, allocator, 1000, "请求未通过验证");
    return true;
  }

  int64_t uid = UserProfileServiceAdapter::Instance().Login(phone, pw, 3);
  if (uid == 2001) {
    LOG_WARN("LoginRequest err : " << params << " code=2001 msg=电话号码未注册");
    ResponseWithJson(ret, allocator, 2001, "电话号码未注册");
    return true;
  } else if (uid == 2002) {
    LOG_WARN("LoginRequest err : " << params << " code=2002 msg=密码错误");
    ResponseWithJson(ret, allocator, 2002, "密码错误");
    return true;
  } else if (uid < 0) {
    LOG_WARN("LoginRequest err : " << params << " code=2000 msg=UserProfileService存在故障");
    ResponseWithJson(ret, allocator, 2000, "服务器故障");
    return true;
  }

  ret.AddMember("userId", uid, allocator);
  LOG_DEBUG("LoginRequest ok : " << params << " uid=" << uid << " code=0 msg=ok"); 
  ResponseWithJson(ret, allocator, 0, "ok");
  return true; 
} // Response

} // namespace fcgi
} // namespace lht
