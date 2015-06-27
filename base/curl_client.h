#ifndef _UTIL_CURL_CLIENT_
#define _UTIL_CURL_CLIENT_

#include <string>

namespace base {

// thread-safe, singleton libcrul client wrapper
//
// update 20150525
// 极光推送JPush提供的推送接口要求两代:
// 1. 在请求时，在接口的HTTP Header中加入验证字段
// 2. 返回的HTTP Response Code作为调用结果编码
// 应此增加GetWithResponse，PostWithResponse，DoRequestWithResponse三个函数
// 函数传入参数加入header，返回结果为response code
class CurlClient {
public:
  ~CurlClient() {}

  static CurlClient & Instance() {
    static CurlClient instance;
    return instance;
  } 
  int Get(const std::string & url, std::string * response);
  int Post(const std::string & url, const std::string & post_body, std::string * response);

  int GetWithResponse(const std::string & url, std::string * response, const std::string * header = NULL);
  int PostWithResponse(const std::string & url, const std::string & post_body, std::string * response, const std::string * header = NULL);
private:
  CurlClient();
  enum HttpMethod {
    HTTP_GET,
    HTTP_POST
  };
  int DoRequest(const std::string & url, HttpMethod method, const std::string & post_body, std::string * response);
  int DoRequestWithResponse(const std::string & url, HttpMethod method, const std::string & post_body, std::string * response, const std::string * header = NULL);
};

}

#endif // _UTIL_CURL_CLIENT_

