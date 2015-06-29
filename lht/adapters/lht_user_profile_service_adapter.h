#ifndef _LHT_USER_PROFILE_SERVICE_ADAPTER_
#define _LHT_USER_PROFILE_SERVICE_ADAPTER_

#include <stdio.h>
#include <map>
#include <set>

#include "LhtUserProfileService.h"

#include "base/config_reader.h"
#include "melon/client/thrift_client_pool.h"
#include "melon/client/fixed_service_locator.h"

using std::map;
using std::set;

namespace lht {

class UserProfileServiceAdapter {
 public:
  static UserProfileServiceAdapter& Instance() {
    static UserProfileServiceAdapter adapter;
    return adapter;
  } 

  std::string SelectEndpoint() {
    return pool_.SelectServer("/lht/lht_user_profile_service");
  }

  int64_t Login(const string& phone, const string& pw, const int max_retry) {
    char parameters[256];
    snprintf(parameters, 255, "phone=%s pw=%s", phone.c_str(), pw.c_str());

    for (int i = 0; i < max_retry; ++i) {
      std::string endpoint = SelectEndpoint();
      LhtUserProfileServiceClient* client = pool_.Alloc(endpoint);
      if (client == NULL) {
        LOG_WARN("UserProfileServiceAdapter Login err : connect err");
        return -100;
      }

      bool success = true;
      int64_t ret = 0;
      try {
        ret = client->Login(phone, pw);
        if (ret < 0) {
          LOG_WARN("UserProfileServiceAdapter Login err : " << parameters);
          success = false;
        }
      } catch (const std::exception& e) {
        LOG_WARN("UserProfileServiceAdapter Login err : " << parameters);
        success = false;
      }
      pool_.Release(endpoint, client, success);
      if (success) {
        LOG_DEBUG("UserProfileServiceAdapter Login ok : " << parameters);
        return ret;
      }
    }
    return -200;
  }

  int SendRegisterVerifyCode(const string& phone, const int max_retry) {
    char parameters[256];
    snprintf(parameters, 255, "phone=%s", phone.c_str());

    for (int i = 0; i < max_retry; ++i) {
      std::string endpoint = SelectEndpoint();
      LhtUserProfileServiceClient* client = pool_.Alloc(endpoint);
      if (client == NULL) {
        LOG_WARN("UserProfileServiceAdapter SendRegisterVerifyCode err : connect err");
        return -100;
      }

      bool success = true;
      int64_t ret = 0;
      try {
        ret = client->SendRegisterVerifyCode(phone);
        if (ret < 0) {
          LOG_WARN("UserProfileServiceAdapter SendRegisterVerifyCode err : " << parameters);
          success = false;
        }
      } catch (const std::exception& e) {
        LOG_WARN("UserProfileServiceAdapter GetRegisterVerfiyCode err : " << parameters);
        success = false;
      }
      pool_.Release(endpoint, client, success);
      if (success) {
        LOG_DEBUG("UserProfileServiceAdapter SendRegisterVerifyCode ok : " << parameters);
        return ret;
      }
    }
    return -200;
  }

  int SendChangePasswordVerifyCode(const string& phone, const int max_retry) {
    char parameters[256];
    snprintf(parameters, 255, "phone=%s", phone.c_str());

    for (int i = 0; i < max_retry; ++i) {
      std::string endpoint = SelectEndpoint();
      LhtUserProfileServiceClient* client = pool_.Alloc(endpoint);
      if (client == NULL) {
        LOG_WARN("UserProfileServiceAdapter SendChangePasswordVerifyCode err : connect err");
        return -100;
      }

      bool success = true;
      int64_t ret = 0;
      try {
        ret = client->SendChangePasswordVerifyCode(phone);
        if (ret < 0) {
          LOG_WARN("UserProfileServiceAdapter SendChangePasswordVerifyCode err : " << parameters);
          success = false;
        }
      } catch (const std::exception& e) {
        LOG_WARN("UserProfileServiceAdapter GetChangePasswordVerfiyCode err : " << parameters);
        success = false;
      }
      pool_.Release(endpoint, client, success);
      if (success) {
        LOG_DEBUG("UserProfileServiceAdapter SendChangePasswordVerifyCode ok : " << parameters);
        return ret;
      }
    }
    return -200;
  }

  int64_t Register(const string& phone, const string& pw, const int32_t code, const int max_retry) {
    char parameters[256];
    snprintf(parameters, 255, "phone=%s pw=%s code=%d", phone.c_str(), pw.c_str(), code);

    for (int i = 0; i < max_retry; ++i) {
      std::string endpoint = SelectEndpoint();
      LhtUserProfileServiceClient* client = pool_.Alloc(endpoint);
      if (client == NULL) {
        LOG_WARN("UserProfileServiceAdapter Register err : connect err");
        return -100;
      }

      bool success = true;
      int64_t ret = 0;
      try {
        ret = client->Register(phone, pw, code);
        if (ret < 0) {
          LOG_WARN("UserProfileServiceAdapter Register err : " << parameters);
          success = false;
        }
      } catch (const std::exception& e) {
        LOG_WARN("UserProfileServiceAdapter Register err : " << parameters);
        success = false;
      }
      pool_.Release(endpoint, client, success);
      if (success) {
        LOG_DEBUG("UserProfileServiceAdapter Register ok : " << parameters);
        return ret;
      }
    }
    return -200;
  }

  int ChangePassword(const string& phone, const string& oldpw, const string& newpw, const int32_t code, const int max_retry) {  
    char parameters[256];
    snprintf(parameters, 255, "phone=%s oldPassword=%s newPassword=%s code=%d", phone.c_str(), oldpw.c_str(), newpw.c_str(), code);

    for (int i = 0; i < max_retry; ++i) {
      std::string endpoint = SelectEndpoint();
      LhtUserProfileServiceClient* client = pool_.Alloc(endpoint);
      if (client == NULL) {
        LOG_WARN("UserProfileServiceAdapter ChangePassword err : connect err");
        return -100;
      }

      bool success = true;
      int64_t ret = 0;
      try {
        ret = client->ChangePassword(phone, oldpw, newpw, code);
        if (ret < 0) {
          LOG_WARN("UserProfileServiceAdapter ChangePassword err : " << parameters);
          success = false;
        }
      } catch (const std::exception& e) {
        LOG_WARN("UserProfileServiceAdapter ChangePassword err : " << parameters);
        success = false;
      }
      pool_.Release(endpoint, client, success);
      if (success) {
        LOG_DEBUG("UserProfileServiceAdapter ChangePassword ok : " << parameters);
        return ret;
      }
    }
    return -200;
  }

 private:
  UserProfileServiceAdapter() : pool_() {
    ConfigReader zk_cfg("../conf/zookeeper.conf");
    if (!zk_cfg) {
      LOG_WARN("Config file ../conf/zookeeper.conf read error!");
      return;
    }
    pool_.set_locator(new melon::client::ZkServiceLocator(zk_cfg.Get("zookeeper", "service_registry").c_str()));
  }

  melon::client::ClientPool<LhtUserProfileServiceClient> pool_;
};

}

#endif // _LHT_USER_PROFILE_SERVICE_ADAPTER_
