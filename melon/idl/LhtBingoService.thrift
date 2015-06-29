include "BaseService.thrift"

namespace cpp lht

service LhtBingoService extends BaseService.BaseService {
  i64 UserEatAtRestaurant(1: i64 userId, 2 : i64 restaurantId);
  set<i64> GetRestaurantListForUser(1: i64 userId);
}
