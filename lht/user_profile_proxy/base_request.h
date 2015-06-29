#ifndef _LHT_SHOPPING_CART_BASE_REQUEST_
#define _LHT_SHOPPING_CART_BASE_REQUEST_

#include "fcgi_service/fcgi_service.h"
#include <string>
#include "rapidjson/document.h"

using ::fcgi::Request; 
using std::string;
using rapidjson::Value;
using rapidjson::Document;

namespace lht {
namespace fcgi {

class BaseRequest : public Request {
public:
  BaseRequest(FCGX_Request* r) : Request(r) {}
  virtual bool Response();
  virtual bool Response(Value& responseValue, Document::AllocatorType& allocator);
protected:
  int ResponseWithJson(Value& responseValue, Document::AllocatorType& allocator);
  int ResponseWithJson(Value& responseValue, Document::AllocatorType& allocator, const int code, const char* msg);
  int VerifyRequest(const string& key, ...);
};

} // namespace fcgi
} // namespace lht

#endif // _LHT_SHOPPING_CART_BASE_REQUEST_

