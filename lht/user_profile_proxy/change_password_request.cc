#include <stdio.h>
#include "base/logging.h"
#include "change_password_request.h"
#include "lht/adapters/lht_user_profile_service_adapter.h"

using namespace std;
using namespace rapidjson;

namespace lht {
namespace fcgi {

bool ChangePasswordRequest::Response(Value& ret, Document::AllocatorType& allocator) {
  const string& phone = GetQuery("phone");
  const string& oldpw = GetQuery("oldPassword");
  const string& newpw = GetQuery("newPassword");
  const int32_t vcode = GetQueryInt("code");
  const string key = GetQuery("key");

  char params[256];
  snprintf(params, 255, "phone=%s oldpw=%s newpw=%s code=%d key=%s", phone.c_str(), oldpw.c_str(), newpw.c_str(), vcode, key.c_str()); 

  LOG_INFO("ChangePasswordRequest receive : " << params); 

  if (VerifyRequest(key, phone.c_str(), oldpw.c_str(), newpw.c_str(), vcode) < 0) {
    LOG_WARN("ChangePasswordRequest err : " << params);
    ResponseWithJson(ret, allocator, 1000, "请求未通过验证");
    return true;
  }

  // TODO 验证用户名、新密码、旧密码格式

  int code = UserProfileServiceAdapter::Instance().ChangePassword(phone, oldpw, newpw, vcode, 3);
  if (code == 2001) {
    LOG_WARN("ChangePasswordRequest err : " << params << " code=2001 msg=电话号码未注册");
    ResponseWithJson(ret, allocator, 2001, "电话号码未注册");
    return true;
  } else if (code == 2002) {
    LOG_WARN("ChangePasswordRequest err : " << params << " code=2002 msg=验证码错误");
    ResponseWithJson(ret, allocator, 2002, "验证码错误");
    return true;
  } else if (code == 2003) {
    LOG_WARN("ChangePasswordRequest err : " << params << " code=2003 msg=密码错误");
    ResponseWithJson(ret, allocator, 2003, "密码错误");
    return true;
  } else if (code < 0) {
    LOG_WARN("ChangePasswordRequest err : " << params << " code=2000 msg=UserProfileService存在故障");
    ResponseWithJson(ret, allocator, 2000, "服务器故障");
    return true;
  }

  LOG_DEBUG("ChangePasswordRequest ok : " << params << " code=" << code << " msg=ok"); 
  ResponseWithJson(ret, allocator, 0, "ok");
  return true; 
} // Response

} // namespace fcgi
} // namespace lht
