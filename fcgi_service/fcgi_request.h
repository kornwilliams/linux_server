#ifndef _FCGI_REQUEST_H_
#define _FCGI_REQUEST_H_

#include <fcgi_stdio.h>
#include <string>
#include <map>
#include <boost/shared_ptr.hpp>

namespace fcgi {

class Request {
public:
  Request(FCGX_Request* r) : fcgi_out_(r->out) {
    Init(r);
  }

  // 取得 HTTP 请求参数
  const std::string & GetParam(const std::string & name) const;

  // 取得 cookie 请求参数
  const std::string & GetCookie(const std::string& key) const;
  int32_t GetCookieInt(const std::string& key, int32_t dft) const;

  // 取得 query string 的请求参数
  const std::string & GetQuery(const std::string& key) const; // 返回 Query 中 key 对应的value
  int32_t GetQueryInt(const std::string& key, int32_t dft = -1) const;
  int64_t GetQueryLong(const std::string& key, int64_t dft = -1) const;

  // 取得请求的body(POST方式)
  std::string GetBody() const;
  // 取得body中的请求参数，部分方式不适用，如文件
  const std::string & GetPostArg(const std::string & key) const;
  int64_t GetPostArgInt(const std::string& key, int64_t dft = -1) const;

  // 取得客户端地址
  std::string GetClientAddr() const;

  virtual bool Response();
  virtual ~Request() {} 
protected:
  FCGX_Stream * fcgi_out_;
  void Init(FCGX_Request* r);

  bool ParseKeyValue(const char * data, const char * key_seps, char kv_sep, std::map<std::string, std::string> * pairs) const;

  std::string body_;
  std::map<std::string, std::string> cookie_map_;
  std::map<std::string, std::string> query_map_;
  mutable std::map<std::string, std::string> body_map_;

  void InitParamMap(FCGX_Request* r);
  // FCGX_GetParam 调用可能出错, fixed
  std::map<std::string, std::string> param_map_;
};
typedef boost::shared_ptr<Request> RequestPtr;


class RequestFactory {
public:
  virtual RequestPtr Create(FCGX_Request* r) = 0;
  virtual ~RequestFactory() {}
};
typedef boost::shared_ptr<RequestFactory> RequestFactoryPtr;

}

#endif // _FCGI_REQUEST_H_
