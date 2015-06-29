#ifndef _LHT_BINGO_SERVICE_ADAPTER_
#define _LHT_BINGO_SERVICE_ADAPTER_

#include "LhtBingoService.h"

#include "base/config_reader.h"
#include "melon/client/thrift_client_pool.h"
#include "melon/client/fixed_service_locator.h"

namespace lht {

class BingoServiceAdapter {
 public:
  static BingoServiceAdapter& Instance() {
    static BingoServiceAdapter adapter;
    return adapter;
  } 

  std::string SelectEndpoint() {
    return pool_.SelectServer("/lht/lht_bingo_service");
  }

  int UserEatAtRestaurant(const int64_t userId, const int64_t restaurantId, const int max_retry) {  
  for (int i = 0; i < max_retry; ++i) {
      std::string endpoint = SelectEndpoint();
      LhtBingoServiceClient* client = pool_.Alloc(endpoint);
      if (client == NULL) {
        LOG_WARN("BingoServiceAdapter UserEatAtRestaurant err : connect err");
        return -1;
      }

      bool success = true;
      int64_t ret = 0;
      try {
        ret = client->UserEatAtRestaurant(userId, restaurantId);
        if (ret != 0) {
          LOG_WARN("BingoServiceAdapter UserEatAtRestaurant err : userId=" << userId << " restaurantId=" << restaurantId);
          success = false;
        }
      } catch (const std::exception& e) {
        LOG_WARN("BingoServiceAdapter UserEatAtRestaurant err : userId=" << userId << " restaurantId=" << restaurantId);
        success = false;
      }
      pool_.Release(endpoint, client, success);
      if (success) {
        LOG_DEBUG("BingoServiceAdapter UserEatAtRestaurant ok : userId=" << userId << " restaurantId=" << restaurantId);
        return ret;
      }
    }
    return -2;
  }

  int GetRestaurantListForUser(set<int64_t>& resSet, const int64_t userId, const int max_retry) {
    for (int i = 0; i < max_retry; ++i) {
      std::string endpoint = SelectEndpoint();
      LhtBingoServiceClient* client = pool_.Alloc(endpoint);
      if (client == NULL) {
        LOG_WARN("BingoServiceAdapter GetRestaurantListForUser err : connect err");
        return -1;
      }

      bool success = true;
      try {
        // TODO 没有返回值的问题如何解决
        client->GetRestaurantListForUser(resSet, userId);
      } catch (const std::exception& e) {
        LOG_WARN("BingoServiceAdapter GetRestaurantListForUser err : userId=" << userId);
        success = false;
      }
      pool_.Release(endpoint, client, success);
      if (success) {
        LOG_DEBUG("BingoServiceAdapter GetRestaurantListForUser ok : userId=" << userId);
        return ret;
      }
    }
    return -2;
  }

 private:
  BingoServiceAdapter() : pool_() {
    ConfigReader zk_cfg("../conf/zookeeper.conf");
    if (!zk_cfg) {
      LOG_WARN("Config file ../conf/zookeeper.conf read error!");
      return;
    }
    pool_.set_locator(new melon::client::ZkServiceLocator(zk_cfg.Get("zookeeper", "service_registry").c_str()));
  }

  melon::client::ClientPool<LhtBingoServiceClient> pool_;
};

}

#endif // _LHT_Bingo_SERVICE_ADAPTER_

