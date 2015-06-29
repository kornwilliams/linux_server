#include "LhtShoppingCartService_constants.h"
#include "LhtShoppingCartService.h"
#include <set>
#include <map>
#include <string>
#include "melon/service/service_arch.h"
#include "melon/service/base_service_impl.h"
#include "base/logging.h"
#include "base/get_local_ip.h"
#include "thread_manager.h"
#include "redis_adapter.h"

using namespace melon::service;
using namespace std;

namespace lht {

class LhtShoppingCartServiceHandler : public LhtShoppingCartServiceIf, public BaseServiceImpl {
 public:
  LhtShoppingCartServiceHandler() {}


  virtual int32_t AddItems(const int64_t userId, const map<int64_t, int32_t>& itemMap) {
    // TODO 添加到购物车用户不用等待结果，另起线程任务
    // TODO 起任务将商品概要加载到缓存
   
    return ShoppingCartRedisAdapter::Instance().AddItems(userId, itemMap);
  }

  virtual int32_t DelItems(const int64_t userId, const set<int64_t>& itemSet) {
    return ShoppingCartRedisAdapter::Instance().DelItems(userId, itemSet);
  }

  virtual int32_t UpdateItems(const int64_t userId, const map<int64_t, int32_t>& itemMap) {
    return ShoppingCartRedisAdapter::Instance().UpdateItems(userId, itemMap);
  }

  virtual void GetItems(map<int64_t, int32_t>& itemMap, const int64_t userId) {
    ShoppingCartRedisAdapter::Instance().GetItems(itemMap, userId);
  }
};

}

int main(int argc, char **argv) {
  char * conf_file = "../conf/lht_shopping_cart_service.conf";
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
  int port = cfg.GetWithType<int>("lht_shopping_cart_service", "port", 19094);

  LOG_INFO("lht_shopping_cart_service start : port=" << port);

  ThriftService<LhtShoppingCartServiceHandler, LhtShoppingCartServiceProcessor> service;
  service.StartRegister("/lht/lht_shopping_cart_service", "1.0", 0, base::GetLocalIp(), port, 1);
  service.Start(port);
  return 0;
}
