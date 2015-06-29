#include <set>
#include "user_eat_at_restaurant_task.h"
#include "base/logging.h"
#include "redis_adapter.h"
#include "adapters/lht_push_service_adapter.h"

using std::set;

namespace lht {

UserEatAtRestaurantTask::UserEatAtRestaurantTask(const int64_t uid, const int64_t rid) : userId(uid), restaurantId(rid) {}

void UserEatAtRestaurantTask::run() {
  LOG_INFO("UserEatAtRestaurantTask receive : userId=" << userId << " restaurantId=" << restaurantId);

  int res = BingoRedisAdapter::Instance().UserEatAtRestaurant(userId, restaurantId);
  if (res <= 0) {
    LOG_DEBUG("UserEatAtRestaurantTask ok : userId=" << userId << " restaurantId=" << restaurantId << " res=" << res << " resean=用户此前已在该店消费过");
    return;
  }

  set<int64_t> resSet;
  res = BingoRedisAdapter::Instance().GetRestaurantListForUser(resSet, userId);
  if (res < 0) {
    LOG_WARN("UserEatAtRestaurantTask err : userId=" << userId << " restaurantId=" << restaurantId << " res=" << res << " reason=redis");
    return;
  }

  // 1 2 3
  // 4 5 6
  // 7 8 9
  switch (restaurantId) {
  case 1 :
    if (resSet.find(2) != resSet.end() && resSet.find(3) != resSet.end() ) {

    }
    if (resSet.find(4) != resSet.end() && resSet.find(7) != resSet.end() ) {

    }
    break;
  case 2 :
    if (resSet.find(1) != resSet.end() && resSet.find(3) != resSet.end() ) {

    }
    if (resSet.find(5) != resSet.end() && resSet.find(8) != resSet.end() ) {

    }
    break;
  case 3 :
    if (resSet.find(2) != resSet.end() && resSet.find(3) != resSet.end() ) {

    }
    if (resSet.find(4) != resSet.end() && resSet.find(7) != resSet.end() ) {

    }
    break;
  case 4 :
    if (resSet.find(2) != resSet.end() && resSet.find(3) != resSet.end() ) {

    }
    if (resSet.find(4) != resSet.end() && resSet.find(7) != resSet.end() ) {

    }
    break;
  case 5 :
    if (resSet.find(2) != resSet.end() && resSet.find(3) != resSet.end() ) {

    }
    if (resSet.find(4) != resSet.end() && resSet.find(7) != resSet.end() ) {

    }
    break;
  case 6 :
    if (resSet.find(2) != resSet.end() && resSet.find(3) != resSet.end() ) {

    }
    if (resSet.find(4) != resSet.end() && resSet.find(7) != resSet.end() ) {

    }
    break;
  case 7 :
    if (resSet.find(2) != resSet.end() && resSet.find(3) != resSet.end() ) {

    }
    if (resSet.find(4) != resSet.end() && resSet.find(7) != resSet.end() ) {

    }
    break;
  case 8 :
    if (resSet.find(2) != resSet.end() && resSet.find(3) != resSet.end() ) {

    }
    if (resSet.find(4) != resSet.end() && resSet.find(7) != resSet.end() ) {

    }
    break;
  case 9 :
    if (resSet.find(2) != resSet.end() && resSet.find(3) != resSet.end() ) {

    }
    if (resSet.find(4) != resSet.end() && resSet.find(7) != resSet.end() ) {

    }
    break;
  }
}

}



