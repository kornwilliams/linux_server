#include <map>
#include "base/logging.h"
#include "get_items_request.h"
#include "lht/adapters/lht_shopping_cart_service_adapter.h"

using namespace std;
using namespace rapidjson;

namespace lht {
namespace fcgi {

bool GetItemsRequest::Response(Value& ret, Document::AllocatorType& allocator) {
  const int64_t uid = GetQueryLong("userId");
  const string key = GetQuery("key");
  LOG_INFO("GetItemsRequest receive : uid=" << GetQuery("userId") << " key=" << key);

  if (VerifyRequest(key, uid) < 0) {
    LOG_WARN("GetItemsRequest err : uid=" << uid << " key=" << key << " code=1000 msg=请求不合法");
    ResponseWithJson(ret, allocator, 1000, "请求不合法");
    return true;
  }

  map<int64_t, int32_t> itemMap;
  if (ShoppingCartServiceAdapter::Instance().GetItems(itemMap, uid, 3) < 0) {
    LOG_WARN("GetItemsRequest err : uid=" << uid << " key=" << key << " code=2000 msg=ShoppingCartService存在故障");
    ResponseWithJson(ret, allocator, 2000, "服务端故障");
    return true;
  }

  Value items(kObjectType);
  char productBuf[128], numBuf[128];
  for (map<int64_t, int32_t>::const_iterator it = itemMap.begin(); it != itemMap.end(); ++it) {
    sprintf(productBuf, "%ld", it->first);
    Value pBuf;
    pBuf.SetString(productBuf, allocator);

    sprintf(numBuf, "%d", it->second);
    Value nBuf;
    nBuf.SetString(numBuf, allocator);
    
    items.AddMember(pBuf.GetString(), nBuf.GetString(), allocator);
    LOG_DEBUG("GetItemsRequest processing : pid=" << productBuf << " num=" << numBuf);
  }
  ret.AddMember("items", items, allocator);

  LOG_DEBUG("GetItemsRequest ok : uid=" << uid << " itemMap.size=" << itemMap.size() << " key=" << key);
  ResponseWithJson(ret, allocator, 0, "ok");
  return true; 
} // Response

} // namespace fcgi
} // namespace lht
