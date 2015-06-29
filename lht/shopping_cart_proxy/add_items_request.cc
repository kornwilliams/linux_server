#include <map>
#include "base/logging.h"
#include "add_items_request.h"
#include "lht/adapters/lht_shopping_cart_service_adapter.h"

using namespace std;
using namespace rapidjson;

namespace lht {
namespace fcgi {

bool AddItemsRequest::Response(Value& ret, Document::AllocatorType& allocator) {
  const int64_t uid = GetQueryLong("userId");
  const int64_t pid = GetQueryLong("items");
  const string key = GetQuery("key");
  LOG_INFO("AddItemsRequest receive : uid=" << GetQuery("userId") << " pid=" << GetQuery("items") << " key=" << key);

  if (VerifyRequest(key, uid, pid) < 0) {
    LOG_WARN("AddItemsRequest err : uid=" << uid << " pid=" << pid << " key=" << key << " code=1000 msg=请求未通过验证");
    ResponseWithJson(ret, allocator, 1000, "请求未通过验证");
    return true;
  }

  map<int64_t, int32_t> itemMap;
  itemMap[pid] = 1;
  if (ShoppingCartServiceAdapter::Instance().AddItems(uid, itemMap, 3) < 0) {
    LOG_WARN("AddItemsRequest err : uid=" << uid << " pid=" << pid << " key=" << key << " code=2000 msg=ShoppingCartService存在故障");
    ResponseWithJson(ret, allocator, 2000, "服务器故障");
    return true;
  }

  LOG_DEBUG("AddItemsRequest ok : uid=" << uid << " pid=" << pid << " key=" << key << " code=0 msg=ok");
  ResponseWithJson(ret, allocator, 0, "ok");
  return true; 
} // Response

} // namespace fcgi
} // namespace lht
