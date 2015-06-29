#include "base/logging.h"
#include "send_change_password_verify_code_request.h"
#include "lht/adapters/lht_user_profile_service_adapter.h"

using namespace std;
using namespace rapidjson;

namespace lht {
namespace fcgi {

bool SendChangePasswordVerifyCodeRequest::Response(Value& ret, Document::AllocatorType& allocator) {
  const string& phone = GetQuery("phone");
  const string& key = GetQuery("key");

  char params[256];
  snprintf(params, 255, "phone=%s key=%s", phone.c_str(), key.c_str()); 

  LOG_INFO("SendChangePasswordVerifyCodeRequest receives : " << params); 

  if (VerifyRequest(key, phone.c_str()) < 0) {
    LOG_WARN("SendChangePasswordVerifyCodeRequest err : " << params);
    ResponseWithJson(ret, allocator, 1000, "请求未通过验证");
    return true;
  }

  int32_t code = UserProfileServiceAdapter::Instance().SendChangePasswordVerifyCode(phone, 3);
  if (code == 2001) {
    LOG_WARN("SendChangePasswordVerifyCodeRequest err : " << params << " code=2001 msg=电话号码未注册");
    ResponseWithJson(ret, allocator, 2001, "电话号码未注册");
    return true;
  } else if (code == 2002) {
    LOG_WARN("SendChangePasswordVerifyCodeRequest err : " << params << " code=2002 msg=短时间重试次数过多");
    ResponseWithJson(ret, allocator, 2002, "短时间重试次数过多");
    return true;
  } else if (code < 0) {
    LOG_WARN("SendChangePasswordVerifyCodeRequest err : " << params << " code=" << code << " msg=UserProfileService存在故障");
    ResponseWithJson(ret, allocator, 2000, "服务器故障");
    return true;
  }

  LOG_DEBUG("SendChangePasswordVerifyCodeRequest ok : " << params << " code=" << code << " msg=ok"); 
  ResponseWithJson(ret, allocator, code, "ok");
  return true; 
} // Response

} // namespace fcgi
} // namespace lht
