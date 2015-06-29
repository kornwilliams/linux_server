#include "LhtUserProfileService_constants.h"
#include "LhtUserProfileService.h"
#include <stdlib.h>
#include <sstream>
#include <string>
#include "melon/service/service_arch.h"
#include "melon/service/base_service_impl.h"
#include "base/logging.h"
#include "base/get_local_ip.h"
#include "redis_adapter.h"
#include "db_adapter.h"
using namespace melon::service;
using namespace std;

namespace lht {

class LhtUserProfileServiceHandler : public LhtUserProfileServiceIf, public BaseServiceImpl {
public:
  LhtUserProfileServiceHandler() {}


  virtual int64_t Login(const string& phone, const string& pw) {
    int64_t uid = 0;

    int32_t ret = UserProfileRedisAdapter::Instance().GetUser(phone, uid);
    if (ret == -2) {
      LOG_WARN("LhtUserProfileService ChangePassword err : phone=" << phone << " result=电话号码未注>册过");
      return 2001;
    }
    else if (ret < 0) {
      LOG_WARN("LhtUserProfileService ChangePassword err : phone=" << phone << " result=获取缓存中用>户编号失败");
      return -1;
    } 
   
    string storedPassword;
    if (UserProfileRedisAdapter::Instance().GetPassword(phone, storedPassword) < 0) {
      LOG_WARN("LhtUserProfileService ChangePassword err : phone=" << phone << " result=获取缓存中的密码失败");
      return -1;
    }  

    if (storedPassword != pw) {
      LOG_WARN("LhtUserProfileService ChangePassword err : phone=" << phone << " result=未通过密码验证");
      return 2002;
    }
  
    return uid;
  }


  virtual int32_t SendRegisterVerifyCode(const string& phone) {
    int ret = UserProfileRedisAdapter::Instance().CheckUser(phone);   
    if (ret == 1) {
      LOG_WARN("LhtUserProfileService SendRegisterVerifyCode err : phone=" << phone << " result=电话号码已注册过");
      return 2001;
    } 

    int32_t code = _GenerateVerifyCode();
    if (UserProfileRedisAdapter::Instance().SetVerifyCode("register_code", phone, code) < 0) {
      LOG_WARN("LhtUserProfileService SendRegisterVerifyCode err : phone=" << phone << " result=redis保存验证码失败");
      return -1;
    }

    // TODO 下发短信
    
    return 0;
  }


  virtual int64_t Register(const string& phone, const string& pw, const int32_t code) {
    stringstream ss;
    ss << "phone=" << phone << " password=" << pw << " code=" << code;

    int64_t ret = UserProfileRedisAdapter::Instance().CheckUser(phone);
    if (ret == 1) {
      LOG_WARN("LhtUserProfileService Register err : " << ss.str() << " result=电话号码已注册过");
      return 2001;
    }

    ret = UserProfileRedisAdapter::Instance().GetVerifyCode("register_code", phone);
    if (ret == -2) {
      LOG_WARN("LhtUserProfileService Register err : " << ss.str() << " result=验证码过期");
      return 2003;
    } else if (ret < 0) {
      LOG_WARN("LhtUserProfileService Register err : " << ss.str() << " result=redis错误");
      return ret;
    }
    if (ret != code) {
      LOG_WARN("LhtUserProfileService Register err : " << ss.str() << " result=验证码不匹配");
      return 2002;
    }


    int64_t uid = 0L;
    ret = UserProfileDBAdapter::Instance().InsertUser(uid, phone, pw);
    if (ret == -3) {
      LOG_WARN("LhtUserProfileService Register err : " << ss.str() << " result=电话号码已注册过");
      return 2001;
    } else if (ret < 0) {
      LOG_WARN("LhtUserProfileService Register err : " << ss.str() << " result=db错误");
      return ret;
    }

    if (UserProfileRedisAdapter::Instance().SetPassword(phone, pw) < 0) {
      LOG_WARN("LhtUserProfileService Register err : " << ss.str() << " result=缓存密码错误");
    }

    if (UserProfileRedisAdapter::Instance().AddUser(phone, uid) < 0) {
      LOG_WARN("LhtUserProfileService Register err : " << ss.str() << " result=缓存uid失败");
    }

    if (UserProfileRedisAdapter::Instance().DelVerifyCode("register_code", phone) < 0) {
      LOG_WARN("LhtUserProfileService Register err : " << ss.str() << " result=删除验证码失败");
    }
   
    return 0;
  }

  virtual int32_t SendChangePasswordVerifyCode(const string& phone) {
    int ret = UserProfileRedisAdapter::Instance().CheckUser(phone);
    if (ret == 0) {
      LOG_WARN("LhtUserProfileService SendChangePasswordVerifyCode err : phone=" << phone << " result=电话号码未注册过");
      return 2001;
    }
    int32_t code = _GenerateVerifyCode();
    if (UserProfileRedisAdapter::Instance().SetVerifyCode("change_password_code", phone, code) < 0) {
      LOG_WARN("LhtUserProfileService SendChangePasswordVerifyCode err : phone=" << phone << " result=redis保存验证码失败");
      return -1;
    }

    // TODO 下发短信
    
    return 0;
  }

  virtual int32_t ChangePassword(const string& phone, const string& oldPassword, const string& newPassword, const int32_t code) {
    stringstream ss;
    ss << "phone=" << phone << " oldPassword=" << oldPassword << " newPassword=" << newPassword << " code=" << code;  

    int32_t ret = UserProfileRedisAdapter::Instance().CheckUser(phone);
    if (ret == 0) {
      LOG_WARN("LhtUserProfileService ChangePassword err : " << ss.str() << " result=用户未注册过");
      return 2001;
    }
    ret = UserProfileRedisAdapter::Instance().GetVerifyCode("change_password_code", phone);
    if (ret < 0) {
      LOG_WARN("LhtUserProfileService ChangePassword err : " << ss.str() << " ret=" << ret);
      return ret;
    }
    if (ret != code) {
      LOG_WARN("LhtUserProfileService ChangePassword err : " << ss.str() << " result=验证码不匹配");
      return 2002;
    }

    string storedPassword;
    if (UserProfileRedisAdapter::Instance().GetPassword(phone, storedPassword) < 0) {
      LOG_WARN("LhtUserProfileService ChangePassword err : " << ss.str() << " result=获取缓存密码失败");
      return -1;
    }

    if (storedPassword != oldPassword) {
      LOG_WARN("LhtUserProfileService ChangePassword err : " << ss.str() << " result=原密码不匹配");
      return 2003;
    }

    // TODO 在db中修改用户密码
    
    if (UserProfileRedisAdapter::Instance().SetPassword(phone, newPassword) < 0) {
      LOG_WARN("LhtUserProfileService ChangePassword err : " << ss.str() << " result=修改缓存密码失败");
    }   
  
    if (UserProfileRedisAdapter::Instance().DelVerifyCode("change_password_code", phone) < 0) {
      LOG_WARN("LhtUserProfileService ChangePassword err : " << ss.str() << " result=删除使用过的验证码失败");
    }
      
    return 0; 
  }

  virtual void GetUserProfile(UserProfile& profile, const int64_t uid) {
    return;
  }

  virtual int32_t UpdateUserProfile(const UserProfile& profile) {
    return 0;
  }

private:
  virtual int _GenerateVerifyCode() {
    static int MIN = 100000, MAX = 999999;
    return rand() % (MAX + 1 - MIN) + MIN;
  }  
};

}

int main(int argc, char **argv) {
  char * conf_file = "../conf/lht_user_profile_service.conf";
  char opt;
  while ((opt = getopt(argc, argv, "c:")) != -1) {
    switch (opt) {
    case 'c':
      conf_file = optarg;
      break;
    default:
      std::cerr << "Unknown option " << optopt << std::endl;
      return 1;
    }
  }

  ConfigReader cfg(conf_file);
  if (!cfg) {
    std::cerr << "Config file " << conf_file << " read error!" << std::endl;
    return 1;
  }

  using namespace lht;
  LOG_INIT(cfg.Get("log4cplus", "file"), cfg.Get("log4cplus", "level"));
  int port = cfg.GetWithType<int>("lht_user_profile_service", "port", 19096);

  LOG_INFO("lht_user_profile_service starts : port=" << port);

  ThriftService<LhtUserProfileServiceHandler, LhtUserProfileServiceProcessor> service;
  service.StartRegister("/lht/lht_user_profile_service", "1.0", 0, base::GetLocalIp(), port, 1);
  service.Start(port);
  return 0;
}
