#ifndef _LHT_SHOPPING_CART_SERVICE_ADAPTER_
#define _LHT_SHOPPING_CART_SERVICE_ADAPTER_

#include <map>
#include <set>

#include "LhtShoppingCartService.h"

#include "base/config_reader.h"
#include "melon/client/thrift_client_pool.h"
#include "melon/client/fixed_service_locator.h"

using std::map;
using std::set;

namespace lht {

class ShoppingCartServiceAdapter {
 public:
  static ShoppingCartServiceAdapter& Instance() {
    static ShoppingCartServiceAdapter adapter;
    return adapter;
  } 

  std::string SelectEndpoint() {
    return pool_.SelectServer("/lht/lht_shopping_cart_service");
  }

  int AddItems(const int64_t userId, const map<int64_t, int32_t>& itemMap, const int max_retry) {
    for (int i = 0; i < max_retry; ++i) {
      std::string endpoint = SelectEndpoint();
      LhtShoppingCartServiceClient* client = pool_.Alloc(endpoint);
      if (client == NULL) {
        LOG_WARN("ShoppingCartServiceAdapter AddItems err : connect err");
        return -1;
      }

      bool success = true;
      int64_t ret = 0;
      try {
        ret = client->AddItems(userId, itemMap);
        if (ret < 0) {
          LOG_WARN("ShoppingCartServiceAdapter AddItems err : userId=" << userId << " itemMap.size=" << itemMap.size());
          success = false;
        }
      } catch (const std::exception& e) {
        LOG_WARN("ShoppingCartServiceAdapter AddItems err : userId=" << userId << " itemMap.size=" << itemMap.size());
        success = false;
      }
      pool_.Release(endpoint, client, success);
      if (success) {
        LOG_DEBUG("ShoppingCartServiceAdapter AddItems ok : userId=" << userId << " itemMap.size=" << itemMap.size());
        return ret;
      }
    }
    return -2;
  }

  int DelItems(const int64_t userId, const set<int64_t>& itemSet, const int max_retry) {
    for (int i = 0; i < max_retry; ++i) {
      std::string endpoint = SelectEndpoint();
      LhtShoppingCartServiceClient* client = pool_.Alloc(endpoint);
      if (client == NULL) {
        LOG_WARN("ShoppingCartServiceAdapter DelItems err : connect err");
        return -1;
      }

      bool success = true;
      int64_t ret = 0;
      try {
        ret = client->DelItems(userId, itemSet);
        if (ret < 0) {
          LOG_WARN("ShoppingCartServiceAdapter DelItems err : userId=" << userId << " itemSet.size=" << itemSet.size());
          success = false;
        }
      } catch (const std::exception& e) {
        LOG_WARN("ShoppingCartServiceAdapter DelItems err : userId=" << userId << " itemSet.size=" << itemSet.size());
        success = false;
      }
      pool_.Release(endpoint, client, success);
      if (success) {
        LOG_DEBUG("ShoppingCartServiceAdapter DelItems ok : userId=" << userId << " itemSet.size=" << itemSet.size());
        return ret;
      }
    }
    return -2;
  }

  int UpdateItems(const int64_t userId, const map<int64_t, int32_t>& itemMap, const int max_retry) {
    for (int i = 0; i < max_retry; ++i) {
      std::string endpoint = SelectEndpoint();
      LhtShoppingCartServiceClient* client = pool_.Alloc(endpoint);
      if (client == NULL) {
        LOG_WARN("ShoppingCartServiceAdapter UpdateItems err : connect err");
        return -1;
      }

      bool success = true;
      int64_t ret = 0;
      try {
        ret = client->UpdateItems(userId, itemMap);
        if (ret < 0) {
          LOG_WARN("ShoppingCartServiceAdapter UpdateItems err : userId=" << userId << " itemMap.size=" << itemMap.size());
          success = false;
        }
      } catch (const std::exception& e) {
        LOG_WARN("ShoppingCartServiceAdapter UpdateItems err : userId=" << userId << " itemMap.size=" << itemMap.size());
        success = false;
      }
      pool_.Release(endpoint, client, success);
      if (success) {
        LOG_DEBUG("ShoppingCartServiceAdapter UpdateItems ok : userId=" << userId << " itemMap.size=" << itemMap.size());
        return ret;
      }
    }
    return -2;
  }

  int GetItems(map<int64_t, int32_t>& itemMap, const int64_t userId, const int max_retry) {  
  for (int i = 0; i < max_retry; ++i) {
      std::string endpoint = SelectEndpoint();
      LhtShoppingCartServiceClient* client = pool_.Alloc(endpoint);
      if (client == NULL) {
        LOG_WARN("ShoppingCartServiceAdapter GetItems err : connect err");
        return -1;
      }

      bool success = true;
      int64_t ret = 0;
      try {
        client->GetItems(itemMap, userId);
        if (ret < 0) {
          LOG_WARN("ShoppingCartServiceAdapter GetItems err : userId=" << userId);
          success = false;
        }
      } catch (const std::exception& e) {
        LOG_WARN("ShoppingCartServiceAdapter GetItems err : userId=" << userId);
        success = false;
      }
      pool_.Release(endpoint, client, success);
      if (success) {
        LOG_DEBUG("ShoppingCartServiceAdapter GetItems ok : userId=" << userId << " itemMap.size=" << itemMap.size());
        return ret;
      }
    }
    return -2;
  }

 private:
  ShoppingCartServiceAdapter() : pool_() {
    ConfigReader zk_cfg("../conf/zookeeper.conf");
    if (!zk_cfg) {
      LOG_WARN("Config file ../conf/zookeeper.conf read error!");
      return;
    }
    pool_.set_locator(new melon::client::ZkServiceLocator(zk_cfg.Get("zookeeper", "service_registry").c_str()));
  }

  melon::client::ClientPool<LhtShoppingCartServiceClient> pool_;
};

}

#endif // _LHT_SHOPPING_CART_SERVICE_ADAPTER_

