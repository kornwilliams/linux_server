#include <sstream>
#include "base_request.h"
#include "base/logging.h"
#include "rapidjson/prettywriter.h"
#include "rapidjson/stringbuffer.h"
#include "rapidjson/document.h"

using namespace rapidjson;
using namespace std;

namespace lht {
namespace fcgi {

bool BaseRequest::Response() {
  if (!fcgi_out_) {
    LOG_WARN("null fcgi_out!");
    return false;
  }

  Value ret(kObjectType);
  Document document;
  Document::AllocatorType& allocator(document.GetAllocator());

  return Response(ret, allocator);
}

bool BaseRequest::Response(Value& ret, Document::AllocatorType& allocator) {
  return true;
}

int BaseRequest::ResponseWithJson(Value& ret, Document::AllocatorType& allocator) {
  rapidjson::StringBuffer strBuff;
  rapidjson::PrettyWriter<rapidjson::StringBuffer> writer(strBuff);
  ret.Accept(writer);

  string output(strBuff.GetString(), strBuff.Size());
  stringstream jsonStream;

  jsonStream << "200 OK\r\nContent-Encoding: UTF-8\r\n\r\n" << output;
  FCGX_PutS(jsonStream.str().c_str(), fcgi_out_);

  return 0;
}

int BaseRequest::ResponseWithJson(Value& ret, Document::AllocatorType& allocator, const int code, const char* msg) {
  ret.AddMember("code", code, allocator);
  ret.AddMember("msg", msg, allocator);
  return ResponseWithJson(ret, allocator);
}

int BaseRequest::VerifyRequest(const string& key, ...) {
  return 0;
}


} // namespace fcgi

} // namespace lht
