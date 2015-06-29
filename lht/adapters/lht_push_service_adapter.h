#ifndef _LHT_PUSH_SERVICE_ADAPTER_
#define _LHT_PUSH_SERVICE_ADAPTER_

#include "LhtPushService.h"

#include "base/config_reader.h"
#include "melon/client/thrift_client_pool.h"
#include "melon/client/fixed_service_locator.h"

namespace lht {

class PushServiceAdapter {
 public:
  static PushServiceAdapter& Instance() {
    static PushServiceAdapter adapter;
    return adapter;
  } 

  std::string SelectEndpoint() {
    return pool_.SelectServer("/lht/lht_push_service");
  }

  int PushNotificationToUser(const string& alias, const string& alert, const string& title, const map<string, string>& extras, const map<string, string>& options, const int max_retry) {
    for (int i = 0; i < max_retry; ++i) {
      std::string endpoint = SelectEndpoint();
      LhtPushServiceClient* client = pool_.Alloc(endpoint);
      if (client == NULL) {
        LOG_WARN("PushServiceAdapter PushNotificationToUser err : connect err");
        return -1;
      }

      bool success = true;
      int64_t ret = 0;
      try {
        ret = client->PushNotificationToUser(alias, alert, title, extras, options);
        if (ret != 0) {
          LOG_WARN("PushServiceAdapter PushNotificationToUser err : alias=" << alias << " alert=" << alert << " title=" << title << " ret=" << ret);
          success = false;
        }
      } catch (const std::exception& e) {
        LOG_WARN("PushServiceAdapter PushNotificationToUser err : alias=" << alias << " alert=" << alert << " title=" << title << " ret=" << ret << " e=" << e.what());
        success = false;
      }
      pool_.Release(endpoint, client, success);
      if (success) {
        LOG_DEBUG("PushServiceAdapter PushNotificationToUser ok : alias=" << alias << " alert=" << alert << " title=" << title);        
        return ret;
      }
    }
    return -2;
  }

  int PushMessageToUser(const string& alias, const string& content, const string& title, const string& type, const map<string, string>& extras, const map<string, string>& options, const int max_retry) {
    for (int i = 0; i < max_retry; ++i) {
      std::string endpoint = SelectEndpoint();
      LhtPushServiceClient* client = pool_.Alloc(endpoint);
      if (client == NULL) {
        LOG_WARN("PushServiceAdapter PushMessageToUser err : connect err");
        return -1;
      }

      bool success = true;
      int64_t ret = 0;
      try {
        ret = client->PushMessageToUser(alias, content, title, type, extras, options);
        if (ret != 0) {
          LOG_WARN("PushServiceAdapter PushMessageToUser err : alias=" << alias << " content=" << content << " title=" << title << " type=" << type << " ret=" << ret << " e=" << e.what());
          success = false;
        }
      } catch (const std::exception& e) {
        LOG_WARN("PushServiceAdapter PushMessageToUser err : alias=" << alias << " content=" << content << " title=" << title << " type=" << type << " ret=" << ret << " e=" << e.what());
        success = false;
      }
      pool_.Release(endpoint, client, success);
      if (success) {
        LOG_DEBUG("PushServiceAdapter PushMessageToUser ok : alias=" << alias << " content=" << content << " title=" << title << " type=" << type);
        return ret;
      }
    }
    return -2;
  }

 private:
  PushServiceAdapter() : pool_() {
    ConfigReader zk_cfg("../conf/zookeeper.conf");
    if (!zk_cfg) {
      LOG_WARN("Config file ../conf/zookeeper.conf read error!");
      return;
    }
    pool_.set_locator(new melon::client::ZkServiceLocator(zk_cfg.Get("zookeeper", "service_registry").c_str()));
  }

  melon::client::ClientPool<LhtPushServiceClient> pool_;
};

}

#endif // _LHT_PUSH_SERVICE_ADAPTER_

