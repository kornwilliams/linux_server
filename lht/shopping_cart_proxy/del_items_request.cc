#include <set>
#include <boost/tokenizer.hpp>
#include "base/logging.h"
#include "del_items_request.h"
#include "lht/adapters/lht_shopping_cart_service_adapter.h"

using namespace std;
using namespace rapidjson;

namespace lht {
namespace fcgi {

bool DelItemsRequest::Response(Value& ret, Document::AllocatorType& allocator) {
  const int64_t uid = GetQueryLong("userId");
  const string itemsStr = GetQuery("items");
  const string key = GetQuery("key");
  LOG_INFO("DelItemsRequest receive : uid=" << GetQuery("userId") << " items=" << itemsStr << " key=" << key);

  if (VerifyRequest(key, uid, itemsStr.c_str()) < 0) {
    LOG_WARN("DelItemsRequest err : uid=" << uid << " items=" << itemsStr << " key=" << key << " code=1000 msg=请求不合法");
    ResponseWithJson(ret, allocator, 1000, "请求不合法");
    return true;
  }

  set<int64_t> itemSet;
  boost::char_separator<char> separator(",");
  boost::tokenizer<boost::char_separator<char> > tokens(itemsStr, separator);
  boost::tokenizer<boost::char_separator<char> >::iterator it;
  for (it = tokens.begin(); it != tokens.end(); ++it) {
    int64_t pid = atol((*it).c_str());
    itemSet.insert(pid);
    LOG_DEBUG("DelItemsRequest processing : uid=" << uid << " pid=" << pid << " size=" << itemSet.size());
  }

  if (ShoppingCartServiceAdapter::Instance().DelItems(uid, itemSet, 3) < 0) {
    LOG_WARN("DelItemsRequest err : uid=" << uid << " items=" << itemsStr << " key=" << key << " code=2000 msg=ShoppingCartService存在故障");
    ResponseWithJson(ret, allocator, 2000, "服务器故障");
    return true;
  }

  LOG_DEBUG("DelItemsRequest ok : uid=" << uid << " items=" << itemsStr << " key=" << key << " code=0 msg=ok");
  ResponseWithJson(ret, allocator, 0, "ok");
  return true; 
} // Response

} // namespace fcgi
} // namespace lht
