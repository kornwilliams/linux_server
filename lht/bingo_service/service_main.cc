#include "LhtBingoService_constants.h"
#include "LhtBingoService.h"
#include <set>
#include <string>
#include "melon/service/service_arch.h"
#include "melon/service/base_service_impl.h"
#include "base/logging.h"
#include "base/get_local_ip.h"
#include "thread_manager.h"
#include "redis_adapter.h"
#include "user_eat_at_restaurant_task.h"

using namespace melon::service;
using namespace std;

namespace lht {

class LhtBingoServiceHandler : public LhtBingoServiceIf, public BaseServiceImpl {
 public:
  LhtBingoServiceHandler() {
    // TODO targetResSet添加参加活动的餐饮店
  }

  virtual int64_t UserEatAtRestaurant(const int64_t userId, const int64_t restaurantId) {
    LOG_DEBUG("LhtBingoServiceHandler UserEatAtRestaurant receive : userId=" << userId << " restaurantId=" << restaurantId);
    if (targetResSet.find(restaurantId) != targetResSet.end()) {
      GetThreadManager()->add(boost::shared_ptr<Runnable>(new UserEatAtRestaurantTask(userId, restaurantId)));
    }
    return 0;
  }

  virtual void GetRestaurantListForUser(set<int64_t>& resSet, const int64_t userId) {
    LOG_DEBUG("LhtBingoServiceHandler GetRestaurantListForUser receive : userId=" << userId); 
    int res = BingoRedisAdapter::Instance().GetRestaurantListForUser(resSet, userId);
    if (res < 0) {
      LOG_WARN("LhtBingoServiceHandler GetRestaurantListForUser err : userId=" << userId << " reason=redis返回值错误 res=" << res);
    } else {
      LOG_DEBUG("LhtBingoServiceHandler GetRestaurantListForUser ok : userId=" << userId << " res=" << res);
    }
  }

 private:
  set<int64_t> targetResSet;

};

}

int main(int argc, char **argv) {
  char * conf_file = "../conf/lht_bingo_service.conf";
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
  int port = cfg.GetWithType<int>("lht_bingo_service", "port", 19090);

  LOG_INFO("lht_bingo_service start : port=" << port);

  ThriftService<LhtBingoServiceHandler, LhtBingoServiceProcessor> service;
  service.StartRegister("/lht/lht_bingo_service", "1.0", 0, base::GetLocalIp(), port, 1);
  service.Start(port);
  return 0;
}
