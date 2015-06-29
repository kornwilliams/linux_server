include "BaseService.thrift"

namespace cpp lht

service LhtShoppingCartService extends BaseService.BaseService {
  i32 AddItems(1 : i64 userId, 2 : map<i64, i32> itemMap);
  i32 DelItems(1 : i64 userId, 2 : set<i64> itemSet);
  i32 UpdateItems(1 : i64 userId, 2 : map<i64, i32> itemMap);
  map<i64, i32> GetItems(1 : i64 userId);
}
