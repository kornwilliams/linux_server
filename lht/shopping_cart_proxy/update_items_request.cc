#include <map>
#include <boost/tokenizer.hpp> 
#include "base/logging.h"
#include "update_items_request.h"
#include "lht/adapters/lht_shopping_cart_service_adapter.h"

using namespace std;
using namespace rapidjson;
using namespace boost;

namespace lht {
namespace fcgi {

bool UpdateItemsRequest::Response(Value& ret, Document::AllocatorType& allocator) {
  const int64_t uid = GetQueryLong("userId");
  const string itemsStr = GetQuery("items");
  const string key = GetQuery("key");
  LOG_INFO("UpdateItemsRequest receive : uid=" << GetQuery("userId") << " items=" << GetQuery("items") << " key=" << key);

  if (VerifyRequest(key, uid, itemsStr.c_str()) < 0) {
    LOG_WARN("UpdateItemsRequest err : uid=" << uid << " items=" << itemsStr << " key=" << key << " code=1000 msg=请求不合法");
    ResponseWithJson(ret, allocator, 1000, "请求不合法");
    return true;
  }

  map<int64_t, int32_t> itemMap;
  boost::char_separator<char> separator(",");
  boost::tokenizer<boost::char_separator<char> > tokens(itemsStr, separator);
  boost::tokenizer<boost::char_separator<char> >::iterator it;
  for (it = tokens.begin(); it != tokens.end(); ) {
    int64_t pid = atol((*it++).c_str());
    
    if (it == tokens.end()) { 
      LOG_WARN("UpdateItemsRequest 参数不成对 : uid=" << uid << " pid=" << pid);
      break;
    }

    int32_t num = atoi((*it++).c_str());
    itemMap[pid] = num;   
    LOG_DEBUG("UpdateItemsRequest processing : pid=" << pid << " num=" << num << " size=" << itemMap.size());
  }

  if (ShoppingCartServiceAdapter::Instance().UpdateItems(uid, itemMap, 3) < 0) {
    LOG_WARN("UpdateItemsRequest err : uid=" << uid << " items=" << itemsStr << " key=" << key << " code=2000 msg=ShoppingCartService存在故障");
    ResponseWithJson(ret, allocator, 2000, "服务器故障");
    return true;
  }

  LOG_DEBUG("UpdateItemsRequest ok : uid=" << uid << " items=" << itemsStr << " key=" << key << " code=0 msg=ok");
  ResponseWithJson(ret, allocator, 0, "ok");
  return true;
} // Response

} // namespace fcgi
} // namespace lht
